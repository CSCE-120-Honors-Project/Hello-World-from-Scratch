#include "vio.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


volatile vio_mmio_registers* vio_regs = (vio_mmio_registers*)VIO_BASE;

// VirtIO queue layout - must be page-aligned for v1
static vio_queue_layout __attribute__((aligned(4096))) vio_queue;

static vio_descriptor* vio_descriptor_table;
static vioqueue_available_ring* available_ring;
static vioqueue_used_ring* used_ring;

static vio_block_request vio_request_header;
static volatile uint8_t vio_request_status;
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
    
    // For VirtIO MMIO v1, use QueuePFN; for v2, use separate address registers
    if (vio_regs->version == 1) {
        // Version 1: Set guest page size and use QueuePFN
        vio_regs->guest_page_size = VIO_PAGE_SIZE;
        vio_regs->queue_align = VIO_PAGE_SIZE;
        
        // For v1, the queue must be in a contiguous region
        // QueuePFN = physical address / page_size
        uint64_t queue_addr = (uint64_t)&vio_queue;
        uint32_t pfn = (uint32_t)(queue_addr / VIO_PAGE_SIZE);
        
        vio_regs->queue_pfn = pfn;
    } else {
        // Version 2: Use separate address registers
        uint64_t descriptor_table_address = (uint64_t)vio_descriptor_table;
        vio_regs->descriptor_table_address_low = (uint32_t)(descriptor_table_address & 0xFFFFFFFF);
        vio_regs->descriptor_table_address_high = (uint32_t)(descriptor_table_address >> 32);

        uint64_t available_ring_address = (uint64_t)&available_ring;
        vio_regs->available_ring_address_low = (uint32_t)(available_ring_address & 0xFFFFFFFF);
        vio_regs->available_ring_address_high = (uint32_t)(available_ring_address >> 32);

        uint64_t used_ring_address = (uint64_t)&used_ring;
        vio_regs->used_ring_address_low = (uint32_t)(used_ring_address & 0xFFFFFFFF);
        vio_regs->used_ring_address_high = (uint32_t)(used_ring_address >> 32);
    }    // Set final status
    vio_regs->device_status |= VIO_DEVICE_STATUS_DRIVER_OK;
    return 0;
}

int vio_read_sector(uint32_t sector, uint8_t* buffer) {
    // Initialize status to non-OK value
    vio_request_status = 0xFF;
    
    // Prepare block request
    vio_request_header.type = VIO_BLOCK_REQUEST_TYPE_READ;
    vio_request_header.reserved = 0;
    vio_request_header.sector = sector;

    // Set up descriptors
    // Descriptor 0: Request header
    vio_descriptor_table[0].address = (uint64_t)&vio_request_header;
    vio_descriptor_table[0].length = sizeof(vio_block_request);
    vio_descriptor_table[0].flags = VIO_DESCRIPTOR_FLAG_NEXT;
    vio_descriptor_table[0].next = 1;

    // Descriptor 1: Data buffer
    vio_descriptor_table[1].address = (uint64_t)buffer;
    vio_descriptor_table[1].length = VIO_SECTOR_SIZE;
    vio_descriptor_table[1].flags = VIO_DESCRIPTOR_FLAG_WRITE | VIO_DESCRIPTOR_FLAG_NEXT;
    vio_descriptor_table[1].next = 2;

    // Descriptor 2: Status byte
    vio_descriptor_table[2].address = (uint64_t)&vio_request_status;
    vio_descriptor_table[2].length = sizeof(uint8_t);
    vio_descriptor_table[2].flags = VIO_DESCRIPTOR_FLAG_WRITE;
    vio_descriptor_table[2].next = 0;

    // Set available ring
    uint16_t available_ring_index = available_ring->index % VIOQUEUE_SIZE;
    available_ring->ring[available_ring_index] = 0;
    
    __sync_synchronize();
    
    // Notify device current available index is updated
    available_ring->index++;

    __sync_synchronize();

    // Notify device to process available buffer
    vio_regs->queue_notification = 0;

    // Wait for the device to process the request
    int timeout = 10000000;
    while (last_used_index == used_ring->index) {
        timeout--;
        if (timeout == 0) {
            return -1; // Timeout
        }
        __sync_synchronize();
    }

    last_used_index = used_ring->index;

    if (vio_request_status != VIO_REQUEST_STATUS_OK) {
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
