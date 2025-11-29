# FAT (File Allocation Table) Notes for CSCE 120 Honors Project

## Table of Contents

- [What is FAT?](#what-is-fat)
- [FAT Variants](#fat-variants)
- [FAT Terminology](#fat-terminology)
- [FAT32 Structure](#fat-structure)
    - [Master Boot Record (MBR)](#master-boot-record-mbr)
    - [Volume ID](#volume-id)
    - [Overall Arrangement](#overall-arrangement)
- [Parsing Directories](#parsing-directories)
- [Cluster Chains](#cluster-chains)

## What is FAT?
FAT (File Allocation Table) is a simple file system architecture widely used in
various storage devices. FAT's defining characteristic is its use of a file
allocation table to keep track of the locations of files on the disk.

## FAT Variants
There are 3 main variants of the FAT file system:
1. **FAT12**: Used in very small storage devices like floppy disks. This is very
complicated to implement.
2. **FAT16**: An improvement over FAT12, used in larger storage devices like
early hard drives and memory cards. This is the simplest to implement.
3. **FAT32**: The most widely used variant, supporting larger storage devices
and files. This is the variant we'll be implementing in the driver.

## FAT Terminology
- **Sector**: The smallest unit of data storage on a disk, typically 512 bytes.
The disk operates at the sector level. Sectors are numbered starting from 0.
- **Logical Block Addressing (LBA)**: A method of addressing sectors on a disk
using a linear numbering scheme, starting from sector 0.
- **Cluster**: A group of one or more sectors that the file system uses as the
smallest unit of allocation for files. Files, no matter how small, occupy at
least one cluster. Sectors in a cluster are located contiguously on the disk.
- **Cluster Chain**: A linked list of clusters that store the data of a file.
The file allocation table maintains these chains. These are used to track files
that span multiple clusters. The directory entry for a file points to the first
cluster in its chain. Then, the FAT is used to find subsequent clusters in the
chain. Clusters in a chain may not be contiguous on the disk.
- **Directory Entry**: A data structure that contains metadata about a file or
directory, including its name, size, starting cluster, and attributes. Directory
entries are stored in special clusters called directories.
- **File Allocation Table (FAT)**: A table that maps each cluster on the disk to
the next cluster in the chain or marks it as free or end-of-file.
- **End of File (EOF)**: A special marker in the FAT indicating that a cluster
is the last one in a file's cluster chain. Typically this is a value of
0x0FFFFFF8 or higher in FAT32.
- **Data Region**: The area of the disk where file and directory data is stored,
following the reserved sectors and FATs. The data section uses clusters to
organize files. The boot sectors and FATs use sectors instead. Cluster numbering
starts at 2 in the data region.
- **Offset**: The position of a byte or data structure within a sector or file,
typically measured in bytes from the start.

## FAT32 Structure
### Master Boot Record (MBR)
At sector 0 of a FAT32 volume, there is the **Master Boot Record (MBR)**, which
contains the partition table and boot code.
This boot code isn't what we're developing---we're developing the BIOS boot code
that loads the OS from the FAT32 image.

The structure of the MBR is as follows:
| Offset (bytes) | Size (bytes) | Description |
|----------------|---------------|-------------|
| 0x0000 (0) | 446 | Boot code (not relevant for the driver) |
| 0x01BE (446) | 64 | Partition table (4 entries of 16 bytes each) |
| 0x01FE (510) | 2 | Boot signature (0x55AA) |

### Partition Table
The partition table consists of up to 4 entries, each 16 bytes long. Partitions
are sections of the disk that can contain different file systems.

Each partition table entry has the following structure:
| Name | Offset (bytes) | Size (bytes) | Description |
|------|----------------|---------------|-------------|
| Boot Flag | 0 | 1 | Indicates if the partition is bootable (0x80) or not (0x00). This can be ignored for the driver. |
| CHS Begin | 1 | 3 | Starting address of the partition in Cylinder-Head-Sector format. This can be ignored for the driver. |
| Partition Type | 4 | 1 | Type of partition (0x0B or 0x0C for FAT32). |
| CHS End | 5 | 3 | Ending address of the partition in Cylinder-Head-Sector format. This can be ignored for the driver. |
| LBA Begin | 8 | 4 | Starting sector of the partition in Logical Block Addressing (LBA) format. |
| Number of Sectors | 12 | 4 | Total number of sectors in the partition. |

### Volume ID
The Volume ID is located at sector 0 of the FAT32 partition (which may not be
sector 0 of the disk if there are multiple partitions). The address of the Volume
ID with respect to the start of the disk is the LBA Begin value from the
partition table.

The Volume ID contains important information about the FAT32 file system, including:
| Name | Official Name | Offset (bytes) | Size (bytes) | Description |
|------|----------------|----------------|---------------|-------------|
| Jump Instruction | BS_jmpBoot | 0x00 (0) | 3 | Jump instruction to boot code (can be ignored for the driver). |
| OEM Name | BS_OEMName | 0x03 (3) | 8 | OEM identifier (can be ignored for the driver). |
| Bytes Per Sector | BPB_BytsPerSec | 0x0B (11) | 2 | Number of bytes per sector (typically 512). |
| Sectors Per Cluster | BPB_SecPerClus | 0x0D (13) | 1 | Number of sectors per cluster (must be a power of 2 up to 128). |
| Reserved Sector Count | BPB_RsvdSecCnt | 0x0E (14) | 2 | Number of reserved sectors before the first FAT (typically 32 for FAT32). |
| Number of FATs | BPB_NumFATs | 0x10 (16) | 1 | Number of FAT copies (typically 2). |
| Sectors Per FAT | BPB_FATSz32 | 0x24 (36) | 4 | Number of sectors occupied by each FAT (varies by disk size). |
| Root Cluster | BPB_RootClus | 0x2C (44) | 4 | Cluster number of the root directory (typically 2). |
| Signature | (none) | 0x1FE (510) | 2 | Boot sector signature (0x55AA). |

The first three fields (Bytes Per Sector, Sectors Per Cluster, Reserved Sector
Count) should be checked to validate the FAT32 file system.
The remaining numbers can be simplified into 4 numbers for accessing the FAT32 filesystem:

1. **FAT Start Sector**: `LBA Begin + Reserved Sector Count`
2. **Cluster Start Sector**: `FAT Start Sector + (Number of FATs * Sectors Per FAT)`
3. **Sectors Per Cluster**: `Sectors Per Cluster`
4. **Root Directory Cluster**: `Root Cluster`

### Overall Arrangement
The overall arrangement of a FAT32 volume is as follows:

1. **Volume ID**: Contains the boot sector and file system metadata.
2. **Reserved Sectors**: Reserved for boot code and other system data.
3. **File Allocation Tables (FATs)**: Multiple copies of the FAT for redundancy.
4. **Data Region**: Contains the actual file and directory data, organized into clusters.
5. **Unused Space**: Any remaining space on the disk not allocated to the file system.

To convert a cluster number to a sector number, use the formula:
`Sector Number = Cluster Start Sector + (Cluster Number - 2) * Sectors Per Cluster`

## Parsing Directories
At the start, only the first cluster of the root directory is known. Reading
this cluster will provide the names and first clusters of other files and directories.
Directories only tell you how to find the first cluster of their contents; you
must read the clusters to get the actual files and subdirectories.
This information comes from the FAT.

> [!NOTE]
> Directories can refer to actual directories or files. Both are represented using
> the same directory entry structure.

Each directory entry is 32 bytes long and contains metadata about a file or
directory. The structure of a directory entry is as follows:
| Name | Official Name | Offset (bytes) | Size (bits) | Description |
|------|----------------|---------------|-------------| -------------|
| Filename | DIR_Name | 0x00 (0) | 88 | Name of the file + extension (padded with spaces). |
| Attributes | DIR_Attr | 0x0B (11) | 8 | File attributes. |
| Reserved | DIR_NTRes | 0x0C (12) | 8 | Reserved for future use. |
| Creation Time (tenths of a second) | DIR_CrtTimeTenth | 0x0D (13) | 8 | Time the file was created in tenths of a second. |
| Creation Time | DIR_CrtTime | 0x0E (14) | 16 | Time the file was created. |
| Creation Date | DIR_CrtDate | 0x10 (16) | 16 | Date the file was created. |
| Last Access Date | DIR_LstAccDate | 0x12 (18) | 16 | Date the file was last accessed. |
| High Cluster | DIR_FstClusHI | 0x14 (20) | 16 | High word of the first cluster number (for FAT32). |
| Last Modified Time | DIR_WrtTime | 0x16 (22) | 16 | Time the file was last modified. |
| Last Modified Date | DIR_WrtDate | 0x18 (24) | 16 | Date the file was last modified. |
| Low Cluster | DIR_FstClusLO | 0x1A (26) | 16 | Low word of the first cluster number. |
| File Size | DIR_FileSize | 0x1C (28) | 32 | Size of the file in bytes. |

The high and low cluster fields combine to form the full starting cluster number
of the file or directory. The low cluster contains bits 0-15, and the high cluster
contains bits 16-31.

The first byte of the filename field can have special values:

- `0x00`: Indicates that there are no more entries in the directory.
- `0xE5`: Indicates that the entry is deleted and can be ignored.
- `0x2E`: Indicates a special entry for the current directory (`.`) or parent
directory (`..`).
- `0x05`: Represents a filename starting with the character `0xE5`.

File attributes can be combined using bitwise OR. Common attributes include:

- `0x01`: Read Only
- `0x02`: Hidden
- `0x04`: System
- `0x08`: Volume Label (only in root directory)
- `0x10`: Subdirectory
- `0x20`: Archive (indicates the file has been modified)
- `0x0F`: Long File Name (LFN) entry

## Cluster Chains
Since directories only give the first cluster of a file or directory, the File
Allocation Table must be used to find the rest of the clusters in the chain.
The FAT is a mix between an array and a linked list. Each entry in the FAT is
a 32-bit value corresponding to a cluster number. The value at that index
indicates the next cluster in the chain or a special marker.

One such marker is 0xFFFFFFFF, which indicates the end of the file (EOF) and
the end of the cluster chain.
> [!CAUTION]
> The end of file marker can be any value from 0x0FFFFFF8 to 0x0FFFFFFF.
> It's usually 0x0FFFFFFF, but it's safer to check for the range.
Another marker is 0x00000000, which indicates that the cluster is free and not
allocated to any file.

> [!NOTE]
> The FAT table only contains data for identifying which clusters belong to
> which files.
> It does not contain any actual file data.
> To read the file data, you must read the clusters indicated by the cluster chain.
> For the `virt` board on QEMU devices, this is done by using the virtio-block
> driver to read sectors from the disk image.
