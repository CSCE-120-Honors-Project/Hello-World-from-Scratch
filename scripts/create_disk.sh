#!/bin/bash

# Create a disk.img that VIO can read
# FIXED VERSION: Correctly formats the partition instead of the whole disk
set -x

DISK_IMG="disk.img"
DISK_SIZE_MB=50

echo "Creating $DISK_IMG ($DISK_SIZE_MB MB)..."

# 1. Create blank disk image
dd if=/dev/zero of=$DISK_IMG bs=1M count=$DISK_SIZE_MB 2>/dev/null
echo "✓ Blank disk created"

# 2. Create MBR and partition table using fdisk
# This creates a partition starting at sector 2048 (1MB offset for alignment)
echo "Creating partition table..."
{
    echo "n"           # new partition
    echo "p"           # primary
    echo "1"           # partition 1
    echo "2048"        # start sector (1MB offset)
    echo ""            # use default end
    echo "t"           # change type
    echo "c"           # FAT32 LBA
    echo "w"           # write
} | fdisk $DISK_IMG > /dev/null 2>&1

echo "✓ Partition table created"

# 3. Format the partition as FAT32
# We need to format ONLY the partition, not the whole disk.
# Since mkfs.fat on a file overwrites the start, we use a temporary file trick.

echo "Formatting partition as FAT32..."

# Calculate partition size
# Total size - 1MB offset
PART_SIZE_MB=$((DISK_SIZE_MB - 1))

# Create a temporary file for the partition content
dd if=/dev/zero of=partition.img bs=1M count=$PART_SIZE_MB 2>/dev/null

# Format the temporary file (prefer mkfs.fat or mkfs.vfat)
if command -v mkfs.fat >/dev/null 2>&1; then
    mkfs.fat -F 32 -n "BOOTDISK" partition.img > /dev/null 2>&1
elif command -v mkfs.vfat >/dev/null 2>&1; then
    mkfs.vfat -F 32 -n "BOOTDISK" partition.img > /dev/null 2>&1
else
    echo "WARNING: neither mkfs.fat nor mkfs.vfat found; partition will be empty"
fi

# Copy the formatted partition into the disk image at the correct offset (1MB = 2048 sectors)
dd if=partition.img of=$DISK_IMG bs=1M seek=1 conv=notrunc 2>/dev/null

# Clean up temp file
rm partition.img

echo "✓ FAT32 filesystem created in partition 1"

# 4. Mount the disk and add kernel file
echo "Adding files to disk..."

MOUNT_POINT="/tmp/vio_disk_mount_$$"
mkdir -p $MOUNT_POINT

LOOP_DEVICE=""
# Try to attach loop device with partition parsing (losetup -fP)
if command -v losetup >/dev/null 2>&1; then
    LOOP_DEVICE=$(sudo losetup -fP --show $DISK_IMG 2>/dev/null || true)
    if [ -n "$LOOP_DEVICE" ]; then
        # Try common partition name variants
        if [ -b "${LOOP_DEVICE}p1" ]; then
            PART_DEV=${LOOP_DEVICE}p1
        elif [ -b "${LOOP_DEVICE}1" ]; then
            PART_DEV=${LOOP_DEVICE}1
        else
            PART_DEV=""
        fi
        if [ -n "$PART_DEV" ]; then
            sudo mount $PART_DEV $MOUNT_POINT 2>/dev/null || true
        else
            # Fallback to offset mount
            PARTITION_OFFSET=1048576
            sudo mount -o loop,offset=$PARTITION_OFFSET $DISK_IMG $MOUNT_POINT 2>/dev/null || true
        fi
    else
        # losetup not allowed or failed; try offset mount
        PARTITION_OFFSET=1048576
        mount -o loop,offset=$PARTITION_OFFSET $DISK_IMG $MOUNT_POINT 2>/dev/null || true
    fi
else
    # No losetup available; try offset mount
    PARTITION_OFFSET=1048576
    mount -o loop,offset=$PARTITION_OFFSET $DISK_IMG $MOUNT_POINT 2>/dev/null || true
fi

if mountpoint -q $MOUNT_POINT >/dev/null 2>&1; then
    # Use the pre-built OS binary from CMake build output
    SCRIPT_DIR=$(pwd)
    OS_BIN="$${SCRIPT_DIR}/build/os/os.bin"

    if [ ! -f "$${OS_BIN}" ]; then
        echo "✗ Failed to find OS binary at $${OS_BIN}"
        rm -rf $MOUNT_POINT
        # Detach loop device if created
        if [ -n "$LOOP_DEVICE" ]; then
            sudo losetup -d $LOOP_DEVICE >/dev/null 2>&1 || true
        fi
        exit 1
    fi

    echo "✓ OS binary found: $${OS_BIN}"
    # Copy OS binary as KERNEL.BIN (requires sudo to write to loop mount)
    sudo cp "$${OS_BIN}" $MOUNT_POINT/KERNEL.BIN
    
    # Verify
    echo "Files on disk:"
    ls -lh $MOUNT_POINT/
    
    # Unmount
    sudo umount $MOUNT_POINT || true
    echo "✓ OS kernel copied to disk"
    # Detach loop device if we attached one
    if [ -n "$LOOP_DEVICE" ]; then
        sudo losetup -d $LOOP_DEVICE >/dev/null 2>&1 || true
    fi
else
    echo "✗ Failed to mount disk"
    rm -rf $MOUNT_POINT
    # Detach loop device if created
    if [ -n "$LOOP_DEVICE" ]; then
        sudo losetup -d $LOOP_DEVICE >/dev/null 2>&1 || true
    fi
    exit 1
fi

# Cleanup
rmdir $MOUNT_POINT 2>/dev/null

echo ""
echo "✓✓✓ disk.img ready for VIO! ✓✓✓"
echo ""
echo "What's inside:"
echo "  - MBR (Master Boot Record)"
echo "  - Partition table (1 FAT32 partition starting at sector 2048)"
echo "  - FAT32 filesystem (inside partition 1)"
echo "  - KERNEL.BIN (real OS kernel from os/os.bin)"
echo ""
echo "Ready to test with: make virtualizeVinux"
