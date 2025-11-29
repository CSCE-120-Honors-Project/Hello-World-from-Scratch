#include "fat.h"
#include "vio.h"

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

static uint32_t fat_begin_lba;
static uint32_t cluster_start_lba;
static uint8_t sectors_per_cluster;
static uint32_t root_cluster;


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
        dest[i] = src[i];
        i++;
    }

    // If there's no extension, finish
    if (src[i] != '.') {
        return;
    }

    i++; // Skip the dot
    for (int j = 8; j < 11 && src[i] != '\0'; j++) {
        dest[j] = src[i++];
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

    // Read the Volume ID sector
    if (vio_read_sector(partition->start_lba, (uint8_t*)&volume_id) < 0) {
        return -1;
    }

    if (volume_id.boot_signature != FAT_BOOT_SIGNATURE) {
        return -1; // Invalid Volume ID signature
    }

    // Initialize FAT32 filesystem parameters
    fat_begin_lba = partition->start_lba + volume_id.reserved_sector_count;
    cluster_start_lba = fat_begin_lba + (volume_id.num_fats * volume_id.fat_size_32);
    sectors_per_cluster = volume_id.sectors_per_cluster;
    root_cluster = volume_id.root_cluster;

    return 0;
}

static int fat_open_r(
        const char* filename, 
        fat_file* file, 
        uint32_t cluster, 
        fat_directory_entry* current_dir
    ) {

    // Zero out current_dir buffer
    for (
            size_t i = 0;
            i < (sectors_per_cluster * FAT_SECTOR_SIZE) / sizeof(fat_directory_entry);
            i++
        ) {
        current_dir[i] = (fat_directory_entry){0};
    }

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
        format_filename((const char*)current_dir[i].name, compare_name);
        if (current_dir[i].name[0] == 0x05) {
            compare_name[0] = (char)0xE5;
        }

        if (filename_compare(compare_name, filename)) {
            // File found
            file->start_cluster = get_cluster(&current_dir[i]);
            file->file_size = current_dir[i].file_size;
            file->current_cluster = file->start_cluster;
            file->is_open = false;

            return 0;
        }

        // File found in recursive call
        if (fat_open_r(
                filename, 
                file, 
                get_cluster(&current_dir[i]), 
                current_dir
            ) == 0) {
            return 0;
        }
    }

    return -1; // File not found
}

int fat_open(const char* filename, fat_file* file) {
    // Current directory buffer used for recursion
    fat_directory_entry current_dir[(FAT_SECTOR_SIZE * sectors_per_cluster) / sizeof(fat_directory_entry)];

    return fat_open_r(
        filename, 
        file, 
        root_cluster, 
        current_dir
    );
}

int fat_read(fat_file* file, uint8_t* buffer) {
    if (!file->is_open) {
        return -1; // File not open
    }
    
    uint32_t current_cluster = file->current_cluster;
    // FAT32 EOC markers are 0x0FFFFFF8 through 0x0FFFFFFF
    while (current_cluster < 0x0FFFFFF8) {
        // Read the current cluster into the buffer
        if (vio_read_sectors(
                cluster_to_lba(current_cluster), 
                sectors_per_cluster, 
                buffer
            ) < 0) {
            return -1; // Read failed
        }

        buffer += cluster_size_bytes();

        // Calculate which sector of the FAT contains the entry for the current cluster
        uint32_t fat_sector_offset = (current_cluster * 4) / FAT_SECTOR_SIZE;
        uint32_t fat_entry_index = current_cluster % (FAT_SECTOR_SIZE / sizeof(uint32_t));

        // Read the specific FAT sector
        // Use uint32_t array to ensure 4-byte alignment
        uint32_t fat_sector[FAT_SECTOR_SIZE / sizeof(uint32_t)];
        if (vio_read_sector(
                fat_begin_lba + fat_sector_offset, 
                (uint8_t*)fat_sector
            ) < 0) {
            return -1; // Read failed
        }
        
        // Get the next cluster from the FAT sector
        current_cluster = fat_sector[fat_entry_index] & 0x0FFFFFFF;
    }

    return 0;
}
