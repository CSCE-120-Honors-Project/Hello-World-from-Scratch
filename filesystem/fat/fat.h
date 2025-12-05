#ifndef FAT_H
#define FAT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FAT_SECTOR_SIZE 512
#define FAT_BOOT_SIGNATURE 0xAA55

#define FAT_PARTITION_TYPE_CHS 0x0B // FAT32 with CHS addressing
#define FAT_PARTITION_TYPE_LBA 0x0C // FAT32 with LBA addressing

typedef struct __attribute__((packed)) {
    uint8_t boot_flag; // 0x00: Boot flag
    uint8_t chs_start[3]; // 0x01 - 0x03: Starting CHS address
    uint8_t type; // 0x04: Partition type (should be 0x0B or 0x0C for FAT32)
    uint8_t chs_end[3]; // 0x05 - 0x07: Ending CHS address
    uint32_t start_lba; // 0x08: Starting LBA address
    uint32_t sectors_count; // 0x0C: Number of sectors in the partition
} fat_partition_entry;



typedef struct __attribute__((packed)) {
    uint8_t boot[446]; // 0x00 - 0x1BD: Reserved boot code
    fat_partition_entry partitions[4]; // 0x1BE - 0x1FD: Partition entries
    uint16_t signature; // 0x1FE - 0x1FF: Boot sector signature (0x55AA)
} fat_master_boot_record;

typedef struct __attribute__((packed)) {
    uint8_t jump_boot[3]; // 0x00 - 0x02: Jump instruction to boot code
    uint8_t oem_name[8]; // 0x03 - 0x0A: Name of manufacturer
    uint16_t bytes_per_sector; // 0x0B - 0x0C: Bytes per sector
    uint8_t sectors_per_cluster; // 0x0D: Sectors per cluster
    uint16_t reserved_sector_count; // 0x0E - 0x0F: Number of reserved sectors
    uint8_t num_fats; // 0x10: Number of FATs (usually 2)
    uint8_t reserved_0[19]; // 0x11 - 0x23: Reserved data not needed for FAT32 driver
    uint32_t fat_size_32; // 0x24 - 0x27: Size of each FAT in sectors
    uint16_t reserved_1[2]; // 0x28 - 0x2B: Reserved data not needed for FAT32 driver
    uint32_t root_cluster; // 0x2C - 0x2F: Root directory starting cluster
    uint8_t reserved_2[462]; // 0x30 - 0x1FD: Reserved data not needed for FAT32 driver
    uint16_t boot_signature; // 0x1FE - 0x1FF: Boot sector signature (0x55AA)
} fat_volume_id;


typedef struct __attribute__((packed)) {
    uint8_t name[11]; // 0x00 - 0x0A: File name (8.3 format without the dot)
    uint8_t attr; // 0x0B: File attributes
    uint8_t reserved; // 0x0C: Reserved for Windows NT
    uint8_t creation_time_tenths; // 0x0D: Creation time in tenths of a second
    uint16_t creation_time; // 0x0E - 0x0F: Creation time
    uint16_t creation_date; // 0x10 - 0x11: Creation date
    uint16_t last_access_date; // 0x12 - 0x13: Last access date
    uint16_t first_cluster_high; // 0x14 - 0x15: High word of first cluster number
    uint16_t write_time; // 0x16 - 0x17: Last write time
    uint16_t write_date; // 0x18 - 0x19: Last write date
    uint16_t first_cluster_low; // 0x1A - 0x1B: Low word of first cluster number
    uint32_t file_size; // 0x1C - 0x1F: File size in bytes
} fat_directory_entry;


typedef struct {
    uint32_t start_cluster;
    uint32_t file_size;
    uint32_t current_cluster;
    bool is_open;
} fat_file;


/**
 * @brief Formats a filename into 8.3 format.
 *
 * This function converts a standard filename into the FAT32 8.3 format,
 * which consists of up to 8 characters for the name and 3 characters for the extension.
 * If the name or extension is shorter than the maximum length, it is padded with spaces.
 * This function can also be used to copy a filename into an 11-byte buffer.
 *
 * @param src The source filename (null-terminated string).
 *  The name should be less than or equal to 8 bytes,
 *  and the extension should be less than or equal to 3 bytes.
 * @param dest The destination buffer (must be at least 11 bytes).
 */
void format_filename(const char* src, char* dest);

/**
 * @brief Initializes the FAT32 filesystem driver.
 *
 * This function must be called before any other FAT operations.
 * It sets up necessary data structures and prepares the driver for use.
 *
 * @return 0 on success, negative value on error.
 */
int fat_init();

/**
 * @brief Mounts a FAT32 partition located at the specified LBA.
 * 
 * This function must be called after fat_init() and before any file operations.
 *
 * @param partition_number The partition number to mount (0-3).
 * @return 0 on success, negative value on error.
 */
int fat_mount(uint8_t partition_number);

/**
 * @brief Opens a file in the FAT32 filesystem.
 *
 * @param filename The name of the file to open (in 8.3 format).
 * @param file Pointer to a fat_file structure to be filled with file information.
 * @return 0 on success, negative value on error (e.g., file not found).
 */
int fat_open(const char* filename, fat_file* file);

/**
 * @brief Reads data from an open FAT32 file.
 *
 * This function reads the entire file starting from the current cluster position
 * and continues through the FAT chain until reaching the end-of-chain marker.
 *
 * @param file Pointer to the fat_file structure representing the open file.
 * @param buffer Pointer to a buffer where the read data will be stored. 
 *               Must be large enough to hold the entire file (at least file_size bytes).
 * @return 0 on success, negative value on error (e.g., I/O error).
 */
int fat_read(fat_file* file, uint8_t* buffer);

#endif