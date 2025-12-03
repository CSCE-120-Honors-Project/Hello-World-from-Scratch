#include "fat.h"
#include "vio.h"
#include "uart.h"

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

static uint32_t fat_begin_lba;
static uint32_t cluster_start_lba;
static uint8_t sectors_per_cluster;
static uint32_t root_cluster;

// ALL buffers static - NEVER on stack
static uint32_t fat_sector_buffer[FAT_SECTOR_SIZE / sizeof(uint32_t)];
static fat_directory_entry dir_cluster_buffer[16];

static inline uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster_start_lba + ((cluster - 2) * sectors_per_cluster);
}

static inline uint32_t cluster_size_bytes() {
    return sectors_per_cluster * FAT_SECTOR_SIZE;
}

static inline int read_dir_cluster(uint32_t cluster, fat_directory_entry* dir_entry) {
    return vio_read_sectors(cluster_to_lba(cluster), sectors_per_cluster, (uint8_t*)dir_entry);
}

static inline uint32_t get_cluster(const fat_directory_entry* entry) {
    return (entry->first_cluster_high << 16) | entry->first_cluster_low;
}

static bool filename_compare(const char* name1, const char* name2) {
    for (int i = 0; i < 11; i++) {
        if (name1[i] != name2[i]) {
            return false;
        }
    }
    return true;
}

void format_filename(const char* src, char* dest) {
    for (int i = 0; i < 11; i++) {
        dest[i] = ' ';
    }

    int i = 0;
    while (i < 8 && src[i] != '\0' && src[i] != '.') {
        dest[i] = src[i];
        i++;
    }

    if (src[i] != '.') {
        return;
    }

    i++;
    for (int j = 8; j < 11 && src[i] != '\0'; j++) {
        dest[j] = src[i++];
    }
}

int fat_init() {
    int result = vio_read_sector(0, (uint8_t*)&mbr);
    
    if (result < 0) {
        return -1;
    }

    if (mbr.signature != FAT_BOOT_SIGNATURE) {
        return -1;
    }

    return 0;
}

int fat_mount(uint8_t partition_number) {
    if (partition_number > 3) {
        return -1;
    }

    fat_partition_entry* partition = &mbr.partitions[partition_number];
    
    if (partition->type != 0x0B && partition->type != 0x0C) {
        return -1;
    }

    int result = vio_read_sector(partition->start_lba, (uint8_t*)&volume_id);
    
    if (result < 0) {
        return -1;
    }

    if (volume_id.boot_signature != FAT_BOOT_SIGNATURE) {
        return -1;
    }

    fat_begin_lba = partition->start_lba + volume_id.reserved_sector_count;
    cluster_start_lba = fat_begin_lba + (volume_id.num_fats * volume_id.fat_size_32);
    sectors_per_cluster = volume_id.sectors_per_cluster;
    root_cluster = volume_id.root_cluster;

    return 0;
}

static int fat_open_r(const char* filename, fat_file* file, uint32_t cluster) {
    if (read_dir_cluster(cluster, dir_cluster_buffer) < 0) {
        return -1;
    }
    
    size_t max_entries = 16;
    
    for (size_t i = 0; i < max_entries; i++) {
        if (dir_cluster_buffer[i].name[0] == 0x00) {
            return -1;
        }

        if ((dir_cluster_buffer[i].attr & 0x0F) == 0x0F) {
            continue;
        }

        if (dir_cluster_buffer[i].name[0] == 0xE5) {
            continue;
        }
        
        char compare_name[11];
        for (size_t j = 0; j < 11; j++) {
            compare_name[j] = dir_cluster_buffer[i].name[j];
        }

        if (dir_cluster_buffer[i].name[0] == 0x05) {
            compare_name[0] = (char)0xE5;
        }

        if (filename_compare(compare_name, filename) && !(dir_cluster_buffer[i].attr & 0x10)) {
            file->start_cluster = get_cluster(&dir_cluster_buffer[i]);
            file->file_size = dir_cluster_buffer[i].file_size;
            file->current_cluster = file->start_cluster;
            file->is_open = true;

            return 0;
        }

        if (dir_cluster_buffer[i].attr & 0x10 && fat_open_r(filename, file, get_cluster(&dir_cluster_buffer[i])) == 0) {
            return 0;
        }
    }

    return -1;
}

int fat_open(const char* filename, fat_file* file) {
    if (file == NULL || filename == NULL) {
        return -1;
    }

    return fat_open_r(filename, file, root_cluster);
}

int fat_read(fat_file* file, uint8_t* buffer) {
    uart_puts("DEBUG fat_read: ENTER\n\r");
    
    if (file == NULL || buffer == NULL) {
        return -1;
    }

    if (!file->is_open) {
        return -1;
    }
    
    uint32_t clusters_read = 0;
    
    while (file->current_cluster < 0x0FFFFFF8 && clusters_read < 1000) {
        if (vio_read_sectors(cluster_to_lba(file->current_cluster), sectors_per_cluster, buffer) < 0) {
            return -1;
        }

        buffer += cluster_size_bytes();
        clusters_read++;

        uint32_t fat_sector_offset = (file->current_cluster * 4) / FAT_SECTOR_SIZE;
        uint32_t fat_entry_index = file->current_cluster % (FAT_SECTOR_SIZE / sizeof(uint32_t));
        
        if (vio_read_sector(fat_begin_lba + fat_sector_offset, (uint8_t*)fat_sector_buffer) < 0) {
            return -1;
        }
        
        file->current_cluster = fat_sector_buffer[fat_entry_index] & 0x0FFFFFFF;
    }

    uart_puts("DEBUG fat_read: EXIT\n\r");
    return 0;
}
