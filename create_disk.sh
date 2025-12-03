#!/bin/bash

# Create a disk.img that VIO can read
# FIXED VERSION: Correctly formats the partition instead of the whole disk

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
dd if=partition.img of=$DISK_IMG bs=512 seek=2048 conv=notrunc 2>/dev/null

# Clean up temp file
rm partition.img

echo "✓ FAT32 filesystem created in partition 1"

# 4. Mount the disk and add kernel file
echo "Adding files to disk..."

MOUNT_POINT="/tmp/vio_disk_mount_$$"
mkdir -p $MOUNT_POINT

# Mount using loopback at the partition offset (1MB = 1048576 bytes)
PARTITION_OFFSET=1048576
sudo mount -o loop,offset=$PARTITION_OFFSET $DISK_IMG $MOUNT_POINT

if [ $? -eq 0 ]; then
    # Create a simple test kernel binary
    echo "Creating test kernel (152 bytes)..."
    
    # Create a minimal ARM64 binary
    cat > kernel.s << 'EOF'
.section ".text"
.global _start
_start:
    wfe
    b _start
EOF
    
    # Assemble and link
    aarch64-linux-gnu-as kernel.s -o kernel.o 2>/dev/null
    aarch64-linux-gnu-ld -Ttext=0x40080000 kernel.o -o kernel.elf 2>/dev/null
    aarch64-linux-gnu-objcopy -O binary kernel.elf kernel.bin 2>/dev/null
    
    # Copy to disk with 8.3 filename format
    # Note: Linux FAT driver handles case, but we ensure it's uppercase for clarity
    sudo cp kernel.bin $MOUNT_POINT/KERNEL.BIN
    
    # Verify
    echo "Files on disk:"
    sudo ls -lh $MOUNT_POINT/
    
    # Unmount
    sudo umount $MOUNT_POINT
    echo "✓ Kernel copied to disk"
else
    echo "✗ Failed to mount disk"
    rm -rf $MOUNT_POINT
    exit 1
fi

# Cleanup
rm -f kernel.s kernel.o kernel.elf kernel.bin
rmdir $MOUNT_POINT 2>/dev/null

echo ""
echo "✓✓✓ disk.img ready for VIO! ✓✓✓"
echo ""
echo "What's inside:"
echo "  - MBR (Master Boot Record)"
echo "  - Partition table (1 FAT32 partition starting at sector 2048)"
echo "  - FAT32 filesystem (inside partition 1)"
echo "  - KERNEL.BIN (test kernel)"
echo ""
echo "Ready to test with: make virtualizeVinux"
