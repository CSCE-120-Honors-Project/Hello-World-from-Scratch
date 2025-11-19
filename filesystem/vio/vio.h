#ifndef VIO_H
#define VIO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define VIO_BASE 0x0A000000
#define VIOQUEUE_SIZE 16
#define VIO_SECTOR_SIZE 512
#define VIO_PAGE_SIZE 4096


#define VIO_MAGIC_VALUE 0x74726976
#define VIO_VERSION 2

#define VIO_DEVICE_STATUS_ACKNOWLEDGE 0x01
#define VIO_DEVICE_STATUS_DRIVER 0x02
#define VIO_DEVICE_STATUS_DRIVER_OK 0x04
#define VIO_DEVICE_STATUS_FEATURES_OK 0x08 
#define VIO_DEVICE_STATUS_DEVICE_NEEDS_RESET 0x40 
#define VIO_DEVICE_STATUS_FAILED 0x80 

#define VIO_FEATURES_PAGE_1 0x0
#define VIO_FEATURES_PAGE_2 0x1

typedef volatile struct __attribute__((packed)) { 
    uint32_t magic_value; // 0x000 - Should be 0x74726976
    uint32_t version; // 0x004 - Should be 2
    uint32_t device_id; // 0x008
    uint32_t vendor_id; // 0x00C
    uint32_t device_features; // 0x010
    uint32_t selected_device_features; // 0x014
    uint32_t reserved_0[2]; // 0x018 - 0x01F
    uint32_t driver_features; // 0x020
    uint32_t selected_driver_features; // 0x024
    uint32_t reserved_1[2]; // 0x028 - 0x02F
    uint32_t selected_queue; // 0x030
    uint32_t queue_maximum_size; // 0x034
    uint32_t selected_queue_size; // 0x038
    uint32_t reserved_2[2]; // 0x03C - 0x043
    uint32_t queue_ready; // 0x044
    uint32_t reserved_3[2]; // 0x048 - 0x04F
    uint32_t queue_notification; // 0x050
    uint32_t reserved_4[3]; // 0x054 - 0x05F
    uint32_t interrupt_status; // 0x060
    uint32_t interrupt_acknowledgement; // 0x064
    uint32_t reserved_5[2]; // 0x068 - 0x06F
    uint32_t device_status; // 0x070
    uint32_t reserved_6[3]; // 0x074 - 0x07F
    uint32_t descriptor_table_address_low; // 0x080 
    uint32_t descriptor_table_address_high; // 0x084
    uint32_t reserved_7[2]; // 0x088 - 0x08F
    uint32_t available_ring_address_low; // 0x090
    uint32_t available_ring_address_high; // 0x094
    uint32_t reserved_8[2]; // 0x098 - 0x09F
    uint32_t used_ring_address_low; // 0x0A0
    uint32_t used_ring_address_high; // 0x0A4
} vio_mmio_registers;


#define VIO_DESCRIPTOR_FLAG_NEXT 0x0001
#define VIO_DESCRIPTOR_FLAG_WRITE 0x0002

typedef struct __attribute__((packed)) { 
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} vio_descriptor;


typedef struct __attribute__((packed)) {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[VIOQUEUE_SIZE];
} vioqueue_available_ring;


typedef struct __attribute__((packed)) {
    uint32_t index;
    uint32_t length;
} vioqueue_used_element;

typedef struct __attribute__((packed)) {
    uint16_t flags;
    uint16_t index;
    vioqueue_used_element ring[VIOQUEUE_SIZE];
} vioqueue_used_ring;


#define VIO_BLOCK_REQUEST_TYPE_READ 0x00
#define VIO_BLOCK_REQUEST_TYPE_WRITE 0x01

#define VIO_REQUEST_STATUS_OK 0x00
#define VIO_REQUEST_STATUS_IO_ERROR 0x01

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
} vio_block_request;


/**
 * @brief Initializes the VIO block device.
 *
 * This function must be called before any other VIO operations.
 * It sets up the device and prepares it for I/O.
 *
 * @return 0 on success, negative value on error.
 */
int vio_init();

/**
 * @brief Reads a single sector from the VIO block device.
 *
 * @param sector The sector number to read.
 * @param buffer Pointer to a buffer of at least VIO_SECTOR_SIZE bytes to receive the data.
 *
 * @return 0 on success, negative value on error (e.g., invalid sector, I/O error).
 * @note The device must be initialized with vio_init() before calling this function.
 */
int vio_read_sector(uint32_t sector, uint8_t* buffer);

/**
 * @brief Reads multiple consecutive sectors from the VIO block device.
 *
 * @param start_sector The first sector number to read.
 * @param sector_count The number of sectors to read.
 * @param buffer Pointer to a buffer of at least (sector_count * VIO_SECTOR_SIZE) bytes to receive the data.
 *
 * @return 0 on success, negative value on error (e.g., invalid sector range, I/O error).
 * @note The device must be initialized with vio_init() before calling this function.
 */
int vio_read_sectors(uint32_t start_sector, uint32_t sector_count, uint8_t* buffer);

#endif
