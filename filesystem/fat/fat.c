#include "fat.h"
#include "vio.h"
#include "../../uart/uart.h"

// Simple memcpy implementation for freestanding environment
static void* memcpy_local(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

static uint32_t fat_begin_lba;
static uint32_t cluster_start_lba;
static uint8_t sectors_per_cluster;
static uint32_t root_cluster;

// Static buffer for reading FAT sectors to avoid stack overflow
static uint32_t fat_sector_buffer[FAT_SECTOR_SIZE / sizeof(uint32_t)];

// Static buffer for directory entries (supports up to 128 sectors per cluster)
// Max size: 128 sectors * 512 bytes / 32 bytes per entry = 2048 entries
#define MAX_DIR_ENTRIES 2048
static fat_directory_entry dir_entry_buffer[MAX_DIR_ENTRIES];


// Helper functions for safe packed struct access

// Safe read of uint32_t from potentially unaligned packed struct
static inline uint32_t read_uint32_packed(const void* ptr) {
    uint32_t value;
    memcpy_local(&value, ptr, sizeof(uint32_t));
    return value;
}

// Safe read of uint16_t from potentially unaligned packed struct
static inline uint16_t read_uint16_packed(const void* ptr) {
    uint16_t value;
    memcpy_local(&value, ptr, sizeof(uint16_t));
    return value;
}

// Helper functions

// Convert a cluster number to its corresponding LBA
static inline uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster_start_lba + ((cluster - 2) * sectors_per_cluster);
}

// Get the size of a cluster in bytes
static inline uint32_t cluster_size_bytes() {
    return sectors_per_cluster * FAT_SECTOR_SIZE;
}

// Read a cluster from disk into a buffer
static inline int read_dir_cluster(uint32_t cluster, fat_directory_entry* dir_entry) {
    return vio_read_sectors(cluster_to_lba(cluster), sectors_per_cluster, (uint8_t*)dir_entry);
}

// Get the starting cluster of a directory entry
static inline uint32_t get_cluster(const fat_directory_entry* entry) {
    return (entry->first_cluster_high << 16) | entry->first_cluster_low;
}

// Compare two filenames
static bool filename_compare(const char* name1, const char* name2) {
    for (int i = 0; i < 11; i++) {
        if (name1[i] != name2[i]) {
            return false;
        }
    }
    return true;
}


// Library functions

// NOTE: This only works for short filenames (8.3 format)
void format_filename(const char* src, char* dest) {
    // Clear destination
    for (int i = 0; i < 11; i++) {
        dest[i] = ' ';
    }

    // Copy name part
    int i = 0;
    while (i < 8 && src[i] != '\0' && src[i] != '.') {
        char c = src[i];
        // normalize to upper-case for FAT short names
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        dest[i] = c;
        i++;
    }

    // If there's no extension, finish
    if (src[i] != '.') {
        return;
    }

    i++; // Skip the dot
    for (int j = 8; j < 11 && src[i] != '\0'; j++) {
        char c = src[i++];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        dest[j] = c;
    }
}

int fat_init() {
    // Read the MBR
    if (vio_read_sector(0, (uint8_t*)&mbr) < 0) {
        return -1;
    }

    if (mbr.signature != FAT_BOOT_SIGNATURE) {
        return -1; // Invalid MBR signature
    }

    return 0;
}

int fat_mount(uint8_t partition_number) {
    if (partition_number > 3) {
        return -1; // Invalid partition number
    }

    fat_partition_entry* partition = &mbr.partitions[partition_number];
    if (partition->type != 0x0B && partition->type != 0x0C) {
        return -1; // Not a FAT32 partition
    }

    // Read start_lba safely using memcpy to avoid unaligned access
    uint32_t partition_start_lba = read_uint32_packed(&partition->start_lba);
    
    // Read the Volume ID sector
    if (vio_read_sector(partition_start_lba, (uint8_t*)&volume_id) < 0) {
        return -1;
    }

    // Read fields safely using memcpy to avoid unaligned access
    uint16_t boot_sig = read_uint16_packed(&volume_id.boot_signature);
    
    if (boot_sig != FAT_BOOT_SIGNATURE) {
        return -1; // Invalid Volume ID signature
    }

    // Read other fields safely
    uint16_t reserved_sector_count = read_uint16_packed(&volume_id.reserved_sector_count);
    uint32_t fat_size_32 = read_uint32_packed(&volume_id.fat_size_32);
    uint32_t root_clust = read_uint32_packed(&volume_id.root_cluster);

    // Initialize FAT32 filesystem parameters
    fat_begin_lba = partition_start_lba + reserved_sector_count;
    cluster_start_lba = fat_begin_lba + (volume_id.num_fats * fat_size_32);
    sectors_per_cluster = volume_id.sectors_per_cluster;
    root_cluster = root_clust;

    return 0;
}

static int fat_open_r(
        const char* filename, 
        fat_file* file, 
        uint32_t cluster
    ) {

    // Use our static directory buffer to avoid stack overflow
    fat_directory_entry* current_dir = dir_entry_buffer;

    // Read the directory entries from the specified cluster
    if (read_dir_cluster(cluster, current_dir) < 0) {
        // Read failed
        return -1;
    }
    
    // Search for the file in the directory entries
    for (
            size_t i = 0; 
            i < (sectors_per_cluster * FAT_SECTOR_SIZE) / sizeof(fat_directory_entry);
            i++
        ) {
        if (current_dir[i].name[0] == 0x00) {
            // No more entries, file not found
            return -1;
        }

        if ((current_dir[i].attr & 0x0F) == 0x0F) {
            // Long file name entry, skip
            continue;
        }

        if (current_dir[i].name[0] == 0xE5) {
            // Deleted file, skip
            continue;
        }
        
        // Check for edge case of 0x05 representing 0xE5
        char compare_name[11];
        // Copy name for comparison
        for (size_t j = 0; j < 11; j++) {
            compare_name[j] = current_dir[i].name[j];
        }

        if (current_dir[i].name[0] == 0x05) {
            compare_name[0] = (char)0xE5;
        }

        if (filename_compare(compare_name, filename) && !(current_dir[i].attr & 0x10)) {
            // File found
            file->start_cluster = get_cluster(&current_dir[i]);
            file->file_size = current_dir[i].file_size;
            file->current_cluster = file->start_cluster;
            file->is_open = true;

            return 0;
        }

        // File found in recursive call
        if (
                current_dir[i].attr & 0x10 && // Is a directory
                fat_open_r(
                    filename, 
                    file, 
                    get_cluster(&current_dir[i])
                ) == 0
            ) {
            return 0;
        }
    }

    return -1; // File not found
}

int fat_open(const char* filename, fat_file* file) {
    if (file == NULL || filename == NULL) {
        return -1; // Invalid parameters
    }

    return fat_open_r(
        filename, 
        file, 
        root_cluster
    );
}

int fat_read(fat_file* file, uint8_t* buffer) {
    if (file == NULL || buffer == NULL) {
        // uart_puts("DEBUG fat_read: NULL parameter\\n\\r");
        return -1; // Invalid parameters
    }

    uint64_t sp_start;
    asm volatile("mov %0, sp" : "=r"(sp_start));
    uint64_t lr_start;
    asm volatile("mov %0, x30" : "=r"(lr_start));
    
    // uart_puts("DEBUG fat_read: SP at start: 0x");
    // uart_print_hex(sp_start);
    // uart_puts(", LR at start: 0x");
    // uart_print_hex(lr_start);
    // uart_puts(", Buffer: 0x");
    // uart_print_hex((uint64_t)buffer);
    // uart_puts(", File struct: 0x");
    // uart_print_hex((uint64_t)file);
    // uart_puts("\\n\\r");

    if (!file->is_open) {
        // uart_puts("DEBUG fat_read: File not open\\n\\r");
        return -1; // File not open
    }
    
    // uart_puts("DEBUG fat_read: Starting read, current_cluster=0x");
    // uart_print_hex(file->current_cluster);
    // uart_puts("\\n\\r");
    
    // FAT32 EOC markers are 0x0FFFFFF8 through 0x0FFFFFFF
    while (file->current_cluster < 0x0FFFFFF8) {
        // uart_puts("DEBUG fat_read: Reading cluster 0x");
        // uart_print_hex(file->current_cluster);
        // uart_puts(" at LBA 0x");
        // uart_print_hex(cluster_to_lba(file->current_cluster));
        // uart_puts("\\n\\r");
        
        // Read the current cluster into the buffer
        if (vio_read_sectors(
                cluster_to_lba(file->current_cluster), 
                sectors_per_cluster, 
                buffer
            ) < 0) {
            // uart_puts("DEBUG fat_read: vio_read_sectors failed\\n\\r");
            return -1; // Read failed
        }

        // uart_puts("DEBUG fat_read: Cluster read successfully\\n\\r");
        buffer += cluster_size_bytes();

        // Calculate which sector of the FAT contains the entry for the current cluster
        uint32_t fat_sector_offset = (file->current_cluster * 4) / FAT_SECTOR_SIZE;
        uint32_t fat_entry_index = file->current_cluster % (FAT_SECTOR_SIZE / sizeof(uint32_t));

        // uart_puts("DEBUG fat_read: Reading FAT sector at LBA 0x");
        // uart_print_hex(fat_begin_lba + fat_sector_offset);
        // uart_puts(", entry index ");
        // uart_print_dec(fat_entry_index);
        // uart_puts("\\n\\r");

        // Read the specific FAT sector into our static buffer
        if (vio_read_sector(
                fat_begin_lba + fat_sector_offset, 
                (uint8_t*)fat_sector_buffer
            ) < 0) {
            // uart_puts("DEBUG fat_read: FAT sector read failed\\n\\r");
            return -1; // Read failed
        }
        
        // uart_puts("DEBUG fat_read: FAT sector read successfully\\n\\r");
        
        // Get the next cluster from the FAT sector
        file->current_cluster = fat_sector_buffer[fat_entry_index] & 0x0FFFFFFF;
        
        // uart_puts("DEBUG fat_read: Next cluster = 0x");
        // uart_print_hex(file->current_cluster);
        // uart_puts("\\n\\r");
    }

    // uart_puts("DEBUG fat_read: Read complete\\n\\r");
    
    uint64_t sp_end;
    asm volatile("mov %0, sp" : "=r"(sp_end));
    uint64_t lr_end;
    asm volatile("mov %0, x30" : "=r"(lr_end));
    
    // uart_puts("DEBUG fat_read: SP at end: 0x");
    // uart_print_hex(sp_end);
    // uart_puts(", LR at end: 0x");
    // uart_print_hex(lr_end);
    // uart_puts("\\n\\r");
    
    return 0;
}