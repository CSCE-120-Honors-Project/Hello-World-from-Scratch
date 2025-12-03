# VirtIO Notes for CSCE 120 Honors Project

## Table of Contents
- [What is VirtIO](#what-is-virtio)
- [Architecture and Communication](#architecture-and-communication)
    - [VirtQueues](#virtqueues)
    - [Communication Flow](#communication-flow)
- [VirtIO Data Structures](#virtio-data-structures)
    - [MMIO Registers](#mmio-registers)
    - [VirtQueue Descriptor](#virtqueue-descriptor)
    - [VirtQueue Available Ring](#virtqueue-available-ring)
    - [VirtQueue Used Ring](#virtqueue-used-ring)
        - [Used Element Structure](#used-element-structure)
        - [Used Ring Structure](#used-ring-structure)
    - [VirtIO Block Request](#virtio-block-request)


## What is VirtIO
VirtIO is a virtualization standard that allows virtual machines to communicate
with virtualized devices like disks, adapters, and I/O devices.
It provides custom drivers that enable efficient communication between the guest
operating system and the host system.

## Architecture and Communication
VirtIO utilizes a front-end and back-end architecture:
- The front-end resides in the guest OS and interacts with the virtual devices.
- The back-end resides in the host OS and manages the actual hardware resources.

The VirtIO base address is the starting memory address where Virtio devices are
mapped in the guest's address space. For QEMU virt boards, this is typically `0x0A000000`.


### VirtQueues
Communication between the front-end and the back-end is done through VirtQueues.
VirtQueues are ring (circular) buffers that have three main parts:

- **Descriptor Table**: Contains descriptors that point to the data buffers.
    - Each descriptor contains information about the physical address, length,
    flags, and next descriptor address.
- **Available Ring**: The guest driver uses this to notify the device which
buffers are available for processing.
- **Used Ring**: The device uses this to notify the guest driver which buffers
have been processed.

> [!NOTE]
> Ring buffers are FIFO data structures that wrap around when they reach the end.
> This allows for continuous data flow.
> However, original data may be overwritten if the buffer is full.

> [!IMPORTANT]
> The device tasks are performed asynchronously and automatically by the VirtIO.
> The guest driver just needs to wait through interrupts or polling to know when
> the device has completed processing. This can be done through a simple while
> loop that checks the used ring for new entries.

### Communication Flow
The communication flow typically follows these steps:
1. The guest driver initializes the buffers and populates the descriptor table.
2. The descriptor indices are added to the available ring.
3. When a request comes in, the driver increments the available ring index and
notifies the device.
4. The device processes the request from the available ring and updates the used
ring with the processed buffer indices.
5. The device sends an interrupt to the guest driver to signal that processing is
complete.
6. The guest driver reads the used ring to retrieve the processed buffers.

## VirtIO Data Structures
### MMIO Registers
VirtIO devices use a set of MMIO registers for configuration and control.

> [!NOTE]
> MMIO (Memory-Mapped I/O) allows devices to be controlled by reading and writing
> to specific memory addresses as if they were regular memory locations.

There are several important MMIO registers:
| Name | Offset | Size | Description |
|------|--------|------|-------------|
| Magic Value | 0x000 | 4 bytes | Identifies the device as a VirtIO device (should be 0x74726976). |
| Version | 0x004 | 4 bytes | Indicates the version of the VirtIO specification (should be 2). |
| Device ID | 0x008 | 4 bytes | Identifies the type of device (e.g., network, block). Should be 2 for a block device. |
| Vendor ID | 0x00C | 4 bytes | Identifies the vendor of the device. |
| Device Features | 0x010 | 4 bytes | Used to coordinate features between the driver and the device. |
| Device Features Select | 0x014 | 4 bytes | Selects which set of device features to read. |
| Driver Features | 0x020 | 4 bytes | Used by the driver to indicate supported features. |
| Driver Features Select | 0x024 | 4 bytes | Selects which set of driver features to write. |
| Queue Select | 0x030 | 4 bytes | Selects the virtqueue to configure. |
| Queue Num Max | 0x034 | 4 bytes | Indicates the maximum size of the selected virtqueue. |
| Queue Num | 0x038 | 4 bytes | The size of the selected virtqueue. |
| Queue Ready | 0x044 | 4 bytes | Indicates whether the selected virtqueue is ready for use. This is set by the driver. |
| Queue Notify | 0x050 | 4 bytes | Used by the driver to notify the device of new buffers in the selected virtqueue. |
| Interrupt Status | 0x060 | 4 bytes | Indicates the interrupt status of the device. |
| Interrupt Acknowledge | 0x064 | 4 bytes | Used by the driver to acknowledge interrupts. |
| Status | 0x070 | 4 bytes | Used by the driver to set the device status. |
| Queue Descriptor Table Address Low | 0x080 | 4 bytes | Lower 32 bits of the physical address of the descriptor table for the selected virtqueue. |
| Queue Descriptor Table Address High | 0x084 | 4 bytes | Upper 32 bits of the physical address of the descriptor table for the selected virtqueue. |
| Queue Driver Address Low | 0x090 | 4 bytes | Lower 32 bits of the physical address of the available ring for the selected virtqueue. |
| Queue Driver Address High | 0x094 | 4 bytes | Upper 32 bits of the physical address of the available ring for the selected virtqueue. |
| Queue Device Address Low | 0x0A0 | 4 bytes | Lower 32 bits of the physical address of the used ring for the selected virtqueue. |
| Queue Device Address High | 0x0A4 | 4 bytes | Upper 32 bits of the physical address of the used ring for the selected virtqueue. |

> [!IMPORTANT]
> When working with hardware, it's important to disable certain compiler optimizations.
> Declaring pointers, structs, and variables as `volatile` ensures that the
compiler always reads from and writes to the actual memory addresses, preventing
it from caching values in registers or optimizing away necessary reads/writes.
> The `__attribute__((packed))` directive is used to prevent the compiler from adding
> any padding between the members of a struct.

### VirtQueue Descriptor
VirtQueue Descriptors point to the data buffers used in communication.
Each descriptor has the following structure:
| Name | Data Type | Description |
|------|-----------|-------------|
| Address | `uint64_t` | Physical address of the data buffer on disk. |
| Length | `uint32_t` | Length of the data buffer in bytes. |
| Flags | `uint16_t` | Flags indicating properties of the descriptor (writable, next) |
| Next | `uint16_t` | Index of the next descriptor in the chain, applicable only if the next flag is set. |

### VirtQueue Available Ring
The Available Ring is used by the guest driver to notify the device of available
buffers.
The structure of the Available Ring is as follows:
| Name | Data Type | Description |
|------|-----------|-------------|
| Flags | `uint16_t` | Flags for the available ring (e.g., whether interrupts are suppressed). |
| Index | `uint16_t` | Index indicating the next available buffer. |
| Ring | `uint16_t[16]` | Array of descriptor indices that are available for processing. |

### VirtQueue Used Ring
The Used Ring is used by the device to notify the guest driver of processed buffers.

#### Used Element Structure
Each entry in the Used Ring has the following structure:
| Name | Data Type | Description |
|------|-----------|-------------|
| Index | `uint32_t` | Index of the start of the used descriptor chain. |
| Length | `uint32_t` | Total length of the data written to the buffer. |

#### Used Ring Structure
The structure of the Used Ring is as follows:
| Name | Data Type | Description |
|------|-----------|-------------|
| Flags | `uint16_t` | Flags for the used ring (e.g., whether interrupts are suppressed). |
| Index | `uint16_t` | The last processed index (essentially a counter for processed requests). |
| Ring | `Used Element[16]` | Array of used elements that have been processed by the device. |


### VirtIO Block Request
VirtIO Block Requests are used to perform read and write operations on block devices.
The structure of a VirtIO Block Request is as follows:
| Name | Data Type | Description |
|------|-----------|-------------|
| Type | `uint32_t` | Type of request (0 for read, 1 for write). |
| Reserved | `uint32_t` | Reserved field, should be set to 0. |
| Sector | `uint64_t` | Sector number on the block device where the operation is to be performed. |

A complete VirtIO Block Request is a descriptor chain consisting of three parts:
1. **Header**: Points to the block request structure.
2. **Data Buffer**: Points to the actual data to be read or written.
3. **Status Byte**: Points to a single byte that indicates the status of the operation
   (0 for success, 1 for failure, 2 for unsupported).
