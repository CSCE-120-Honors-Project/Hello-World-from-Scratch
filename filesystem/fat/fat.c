#include "fat.h"
#include "vio.h"

static fat_master_boot_record mbr;
static fat_volume_id volume_id;

int fat_init() {
    // Read the MBR
    if (vio_read_sector(0, (uint8_t*)&mbr) < 0) {
        return -1;
    }

    return 0;
}
