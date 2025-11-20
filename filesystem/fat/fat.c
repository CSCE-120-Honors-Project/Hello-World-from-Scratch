#include "fat.h"
#include "vio.h"

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

// TODO: Initialize FAT32 filesystem parameters and the FAT
static unsigned long fat_begin_lba;
static unsigned long cluster_start_lba;
static uint8_t sectors_per_cluster;
static unsigned long root_cluster;

inline uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster_start_lba + ((cluster - 2) * sectors_per_cluster);
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
