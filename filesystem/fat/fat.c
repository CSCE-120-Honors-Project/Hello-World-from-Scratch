#include "fat.h"
#include "vio.h"

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

static unsigned long fat_begin_lba;
static unsigned long cluster_start_lba;
static uint8_t sectors_per_cluster;
static unsigned long root_cluster;


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
static inline int read_cluster(uint32_t cluster, fat_directory_entry* dir_entry) {
    return vio_read_sectors(cluster_to_lba(cluster), sectors_per_cluster, (uint8_t*)dir_entry);
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
    if (partition->type != 0x0B || partition->type != 0x0C) {
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

int fat_open(const char* filename, fat_file* file) {
    char current_filename[11] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    uint32_t current_cluster = root_cluster;
    // Copy filename for comparison

    while(!filename_compare(filename, current_filename)) {
        fat_directory_entry dir_entry;
        // TODO: Unfuck this to actually iterate through directory entries
        // Use what Perplexity explained to help with this
        read_cluster(current_cluster, &dir_entry);
        format_filename((const char*)dir_entry.name, current_filename);
    }

    return 0;
}
