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

# Format the temporary file
mkfs.fat -F 32 -n "BOOTDISK" partition.img > /dev/null

# Copy the formatted partition into the disk image at the correct offset (1MB = 2048 sectors)
dd if=partition.img of=$DISK_IMG bs=1M seek=1 conv=notrunc 2>/dev/null

# Clean up temp file
rm partition.img

echo "✓ FAT32 filesystem created in partition 1"

# 4. Mount the disk and add kernel file
echo "Adding files to disk..."

MOUNT_POINT="/tmp/vio_disk_mount_$$"
mkdir -p $MOUNT_POINT

# Mount using loopback at the partition offset (1MB = 1048576 bytes)
PARTITION_OFFSET=1048576
mount -o loop,offset=$PARTITION_OFFSET $DISK_IMG $MOUNT_POINT

if [ $? -eq 0 ]; then
    # Build the real OS
    echo "Building OS kernel..."
    
    # Save current directory
    SCRIPT_DIR=$(pwd)
    
    # Build OS
    cd os
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
    
    if [ ! -f os.bin ]; then
        echo "✗ Failed to build OS"
        cd $SCRIPT_DIR
        umount $MOUNT_POINT
        rm -rf $MOUNT_POINT
        exit 1
    fi
    
    echo "✓ OS built successfully"
    
    # Copy OS binary as KERNEL.BIN
    cp os.bin $MOUNT_POINT/KERNEL.BIN
    
    # Return to original directory
    cd $SCRIPT_DIR
    
    # Verify
    echo "Files on disk:"
    ls -lh $MOUNT_POINT/
    
    # Unmount
    umount $MOUNT_POINT
    echo "✓ OS kernel copied to disk"
else
    echo "✗ Failed to mount disk"
    rm -rf $MOUNT_POINT
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
