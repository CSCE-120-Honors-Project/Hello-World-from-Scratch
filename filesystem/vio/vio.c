#include "vio.h"
#include "../../uart/uart.h"

volatile vio_mmio_registers* vio_regs = (vio_mmio_registers*)VIO_BASE;

// VirtIO queue layout - must be page-aligned for v1
static vio_queue_layout __attribute__((aligned(4096))) vio_queue;

static vio_descriptor* vio_descriptor_table;
static vioqueue_available_ring* available_ring;
static volatile vioqueue_used_ring* used_ring;

static vio_block_request vio_request_header;
static volatile uint8_t vio_request_status;

// Track which used ring entry we've processed up to
static uint16_t last_used_index = 0;

int vio_init() {
    // Scan MMIO slots to find VirtIO block device
    for (uint64_t addr = 0x0A000000; addr < 0x0A000000 + 0x200 * 32; addr += 0x200) {
        volatile vio_mmio_registers* regs = (vio_mmio_registers*)addr;
        if (regs->magic_value == VIO_MAGIC_VALUE &&
            regs->device_id != 0 &&
            (regs->version == 1 || regs->version == 2)) {
            vio_regs = regs;
            break;
        }
    }

    // Check if it's a VirtIO device
    if (vio_regs->magic_value != VIO_MAGIC_VALUE) {
        return -1; // Not a VirtIO device
    }

    if (vio_regs->version != 1 && vio_regs->version != 2) {
        return -1; // Unsupported VirtIO version
    }

    // Reset device
    vio_regs->device_status = 0;

    // Accept default device and driver features
    vio_regs->device_status |= VIO_DEVICE_STATUS_ACKNOWLEDGE;
    vio_regs->device_status |= VIO_DEVICE_STATUS_DRIVER;

    // Negotiate features (legacy VirtIO doesn't require VIRTIO_F_VERSION_1)
    vio_regs->selected_device_features = 0;
    vio_regs->selected_driver_features = 0;
    vio_regs->device_status |= VIO_DEVICE_STATUS_FEATURES_OK;

    if (!(vio_regs->device_status & VIO_DEVICE_STATUS_FEATURES_OK)) {
        vio_regs->device_status |= VIO_DEVICE_STATUS_FAILED;
        return -1; // Device did not accept features
    }

    vio_regs->selected_queue = 0;

    if (vio_regs->queue_maximum_size < 16) {
        return -1; // Queue too small
    }

    vio_regs->selected_queue_size = VIOQUEUE_SIZE;

    // Set up queue layout pointers
    vio_descriptor_table = vio_queue.descriptors;
    available_ring = &vio_queue.available;
    used_ring = &vio_queue.used;

    // Set guest page size BEFORE setting PFN
    vio_regs->guest_page_size = VIO_PAGE_SIZE;

    // For VirtIO v1: Pass physical page frame number
    // In bare metal, physical == virtual, so just divide by page size
    uint64_t queue_addr = (uint64_t)&vio_queue;
    uint32_t pfn = (uint32_t)(queue_addr / VIO_PAGE_SIZE);
    vio_regs->queue_pfn = pfn;

    // Set final status
    vio_regs->device_status |= VIO_DEVICE_STATUS_DRIVER_OK;

    return 0;
}

int vio_read_sector(uint32_t sector, uint8_t* buffer) {
    // Initialize status to non-OK value
    vio_request_status = 0xFF;

    // Memory barrier before preparing the request
    __sync_synchronize();

    // Prepare block request
    vio_request_header.type = VIO_BLOCK_REQUEST_TYPE_READ;
    vio_request_header.reserved = 0;
    vio_request_header.sector = sector;

    // Set up descriptors (chain: request -> data -> status)
    // Descriptor 0: Request header (read-only)
    vio_descriptor_table[0].address = (uint64_t)&vio_request_header;
    vio_descriptor_table[0].length = sizeof(vio_block_request);
    vio_descriptor_table[0].flags = VIO_DESCRIPTOR_FLAG_NEXT;
    vio_descriptor_table[0].next = 1;

    // Descriptor 1: Data buffer (written by device)
    vio_descriptor_table[1].address = (uint64_t)buffer;
    vio_descriptor_table[1].length = VIO_SECTOR_SIZE;
    vio_descriptor_table[1].flags = VIO_DESCRIPTOR_FLAG_WRITE | VIO_DESCRIPTOR_FLAG_NEXT;
    vio_descriptor_table[1].next = 2;

    // Descriptor 2: Status byte (written by device)
    vio_descriptor_table[2].address = (uint64_t)&vio_request_status;
    vio_descriptor_table[2].length = sizeof(uint8_t);
    vio_descriptor_table[2].flags = VIO_DESCRIPTOR_FLAG_WRITE;
    vio_descriptor_table[2].next = 0;

    // Memory barrier before notifying device
    __sync_synchronize();

    // Add to available ring
    uint16_t available_ring_index = available_ring->index % VIOQUEUE_SIZE;
    available_ring->ring[available_ring_index] = 0;  // Descriptor chain starts at 0

    // Save expected index BEFORE submitting the request
    uint16_t expected_used_index = used_ring->index + 1;
    
    // Memory barrier before incrementing available index
    __sync_synchronize();

    // Notify device that new descriptor is available
    available_ring->index++;

    // Memory barrier before kicking device
    __sync_synchronize();

    // Kick the device - for v1 MMIO, write queue number to queue_notification
    vio_regs->queue_notification = 0;

    // Wait for the device to process THIS request
    int timeout = 10000000;
    while (timeout > 0) {
        // Memory barrier to ensure we see device updates
        __sync_synchronize();
        
        // Check if device has processed our request
        if (used_ring->index >= expected_used_index) {
            // Device has completed our request
            last_used_index = used_ring->index;
            
            // CRITICAL: Memory barrier after device completion to ensure
            // buffer and status updates are visible to the CPU
            __sync_synchronize();
            break;
        }
        timeout--;
    }

    if (timeout == 0) {
        return -1; // Timeout waiting for device
    }

    // Acknowledge interrupt if one is pending
    if (vio_regs->interrupt_status) {
        vio_regs->interrupt_acknowledgement = vio_regs->interrupt_status;
    }
    
    // Another memory barrier to ensure all device writes are visible
    __sync_synchronize();

    // Check status byte written by device
    if (vio_request_status != VIO_REQUEST_STATUS_OK) {
        uart_puts("I/O error from device\n");
        return -1; // Read failed
    }

    return 0;
}

int vio_read_sectors(uint32_t start_sector, uint32_t sector_count, uint8_t* buffer) {
    for (uint32_t i = 0; i < sector_count; i++) {
        uint8_t* sector_buffer = buffer + (i * VIO_SECTOR_SIZE);
        if (vio_read_sector(start_sector + i, sector_buffer) != 0) {
            return -1;
        }
    }
    return 0;
}
