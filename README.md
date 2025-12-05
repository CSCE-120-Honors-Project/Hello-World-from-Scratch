# Hello World from Scratch - Project Documentation

## Table of Contents
* [Project Description](#project-description)
    * [Project Goals](#project-goals)
    * [Project Constraints](#project-constraints)
    * [Project Contents](#project-contents)
* [Project Code](#project-code)
* [Running the Code](#running-the-code)
* [User Instructions](#user-instructions)
* [Development Reflections](#development-reflections)
    * [Challenging Milestones](#challenging-milestones)
    * [AI Use](#ai-use)
    * [Testing Process](#testing-process)

## Project Description
---
This project is a "Hello, World!" program, with a twist! Typically, even with
simple programs like this, developers rely on standard libraries and
conveniences provided by the operating system, bootloader, or firmware to
facilitate development. This project explores the question of "what if we had to
write a 'Hello, World!' program without any of those conveniences?"

### Project Goals
More specifically, we aimed to write a program that:
* Read a binary that prints "Hello, World!" to the console from a hard drive
* Loaded that binary into memory
* Set up the system environment to run that binary
* Executed the binary to print "Hello, World!" to the console

### Project Constraints
Our constraints were defined by the limited environment of the bare metal system
we chose: a Cortex A53 CPU running on a QEMU virt board. This meant:
* No standard library
* No operating system to handle:
    * Stack initialization
    * Heap memory (no `malloc`/`free` or `new`/`delete`)
    * Loading programs into memory
    * Standard output (no `printf`, `cout`, etc.)
* No file system access


### Project Contents
The combination of our goals and these constraints dictated what our project implemented:
* A UART driver to handle console output
* A VirtIO driver to read from a virtual hard drive
* A FAT32 driver to parse the virtual hard drive to find and read the "Hello,
World!" binary
* The "Hello, World!" binary, which uses the UART driver to print to the console
* The bootloader, which initializes the stack and drivers, loads the binary
into memory, and executes it
* CMake build scripts to compile and link everything together
* A Makefile to build the program and run it in QEMU

## Project Code
---
The source code for this project is here! This very Git repository contains all
of our handcrafted, fair-trade, organically-sourced, free-range, non-GMO,
grass-fed artisan code.

For those who really want a link to the code, [here it is](https://github.com/CSCE-120-Honors-Project/DIY-Bootloader).


## Running the Code

This section assumes you are starting from a completely new machine that only has this repository cloned locally, and no ARM cross-compilers, QEMU, or CMake installed yet. [file:1]

### 1. Prerequisites

Install the following tools:


- CMake (3.20 or newer recommended) 
- GNU Make 
- QEMU with AArch64 support (often provided by `qemu-system-aarch64`) 
- An AArch64 bare‑metal GCC toolchain (for example, `aarch64-none-elf-gcc` or `aarch64-elf-gcc`)

#### Windows (with WSL recommended)

On Windows, the project is easiest to build inside WSL (Ubuntu) while using a Windows‑installed ARM toolchain and QEMU, which the CMake configuration is designed to auto‑detect.
    1. Install WSL and Ubuntu from the Microsoft Store. 
    2. Inside Ubuntu, install build tools:
        ```bash
        sudo apt update
        sudo apt install build-essential cmake qemu-system-aarch64
        ```
    3. Download and install the AArch64 bare-metal GCC toolchain from [Arm's official website](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Look for the `aarch64-none-elf` version for your platform.
    4. Extract the toolchain and add its `bin` directory to your `PATH`:
        ```bash
        export PATH=$PATH:/path/to/arm-gnu-toolchain/bin
        ```
    5. Verify the toolchain is installed:
        ```bash
        aarch64-none-elf-gcc --version
        ```



### Quick Start
```bash
git clone https://github.com/CSCE-120-Honors-Project/DIY-Bootloader.git
cd DIY-Bootloader
make build
make run
```

Press `Ctrl+A` then `X` to exit QEMU.

### What's Included
The project includes a pre-made `disk.img` with:
- FAT32 filesystem
- `KERNEL.BIN` containing the "Hello World" OS
- Already formatted and ready to use

### Updating the OS
If you modify `os/main.c` or other OS files, follow these steps:

1. **Rebuild the OS**:
   ```bash
   make build
   ```

2. **Mount the disk image and update KERNEL.BIN**:

   **On Linux/WSL**:
   ```bash
   mkdir -p /tmp/disk_mount
   sudo mount -o loop disk.img /tmp/disk_mount
   sudo cp build/os/os.bin /tmp/disk_mount/KERNEL.BIN
   sudo umount /tmp/disk_mount
   ```

   **On macOS**:
   ```bash
   hdiutil mount disk.img
   cp build/os/os.bin /Volumes/*/KERNEL.BIN
   hdiutil unmount /Volumes/*
   ```

   **Note**: Loop device support varies by platform. If you encounter issues:
   - **Windows WSL2**: May have loop device limitations; use the macOS approach with `hdiutil` if available
   - **WSL1**: Consider upgrading to WSL2 or using a Linux VM
   - **Docker**: Can mount inside a container with appropriate privileges

3. **Run the updated image**:
   ```bash
   make run
   ```

## User Instructions
---

 Run the Makefile:
        ```bash
        make run
        ```


Remeber to see the output you must view virtual serial port by clicking "view" then "serialport0" 


<img width="340" height="478" alt="image" src="https://github.com/user-attachments/assets/3ee523ee-0a6c-47a7-9935-98c5d8791d2f" />

Final Output should look like this!
<img width="445" height="133" alt="image" src="https://github.com/user-attachments/assets/a7f25908-1175-41ab-bc60-01af4a5a9de1" />



## Development Reflections
---

### Challenging Milestones
In all fairness, the entirety of this project was challenging! At its start,
none of us had any clue how to even begin writing a bootloader from scratch. As
we progressed, we were able to flesh out our understanding of the necessary
components required for the bootloader, at the cost of a bulk of the work for
the project being towards the end once we understood what we had to do in the
first place. However, there were some milestones that stood out as particularly
challenging:

* **VirtIO Driver Implementation**: Initially, we were under the impression that
the VirtIO functionality was baked-into QEMU and was a simple drop-in for our
FAT32 driver to use. This was not the case. To boot, QEMU uses the legacy VirtIO
version 1, which wasn't made apparent in the documentation we found. This
resulted in us writing a VirtIO version 2 driver for the virtual hard drive,
only to find out it didn't work. The subsequent rewrite was conducted by AI,
which posed its own challenges (see below).
* **CMake Configuration**: Beyond simple Makefiles, none of us had experience
in using C/C++ build tools. CMake is already known for being notoriously unintuitive.
To boot, we had to use a custom ARM toolchain (GCC for ARM 64-bit bare metal)
to build our project, which added another set of things to configure. To make
matters even worse, Kedar has a Macbook for development while Dannie and Ved
had Windows + WSL. Different ARM toolchains had to be used on each platform to
successfully cross-compile for an AArch64 Cortex A53 CPU. This meant our CMake
file had to be aware of which platform it was being built on (and understand
that WSL is running using Windows tools and not Linux ones) to successfully build
our project.
* **Final Deployment**: QEMU has many command-line options to configure the
virtual machine, and these are often confusingly named. Understanding that a
flag like `-bios` is **not** for specifying a second-stage bootloader, but the
`-kernel` flag is, lead to much confusion. Learning that QEMU may not always
mount VirtIO devices to the same MMIO register was another quirk we had to work
around, with the solution being cleverly ordering QEMU's command-line arguments
to guarantee an MMIO address location. Understanding these command-line options
was essential for creating our final Makefile that built the project (using
CMake) and ran it in QEMU. This setup also made it hard to communicate to others
what it was doing: "Our Makefile builds our project using CMake, which makes a
Makefile and recursively triggers other CMake builds that make more Makefiles.
And then it runs it in QEMU." (try saying that five times fast!)

### AI Use
AI was a massive help for this project. This was our first time writing
low-level, bare metal code, and AI made it far easier to parse the hundreds of
pages of documentation and specification for Cortex A53 and VirtIO. When we had
to pivot our VirtIO driver, AI generated almost all of the new code for us,
saving a lot of time. As described later, AI also wrote our entire driver test
suite, which helped us automate validation of both human-written and
AI-generated code.

AI was also instrumental in catching bugs and issues in our code. Any time we
wanted to merge new code into our main branch, we would open pull requests and
request GitHub Copilot as a reviewer. Copilot is an extremely pedantic and
thorough reviewer, which lead to us catching many small, hard-to-find bugs and
edge cases we didn't even know to consider.

That being said, AI-generated code did come with its issues. While the AI coding
agents could ingest the various specifications and documentation we threw at it,
it wasn't great at actually implementing the code correctly.
This led to many headaches in trying to debut the AI-generated VirtIO driver,
as it was clear neither us nor the AI understood what we were doing.
> As an aside, we tried several different prompt engineering strategies, with
> varying degrees of success. We found that (obviously) vague "please fix"
> requests had the least amount of success. Surprisingly, specific and detailed
> prompts with good prompt engineering techniques (e.g. few-shot prompting) had
> the next worst success rate. The best results came from very unorthordox
> prompting techniques. Promising to call the AI "good boy" upon successful code
> generation had the second-highest success rate. The best success rate came from
> berating the code agent with expletives and insults (i.e., "your inane
> solutions pollute the environment and waste water"). The most progress was
> made when the AI was insulted.
>
> As an aside to an aside, it was also interesting to see the AI agent's tone
> shift after we insulted it.
> The AI responses became more informal and casual, like it was trying to please
> us with humor after being viciously insulted.
>
> This isn't a joke. We were desperate enough to try this.

Our test suit was also AI-generated, but it was difficult to tell whether the
errors it raised were due to bugs in our code or mistakes in the test suite itself.
The test suite was intended to validate the three drivers we wrote, ensuring that
their individual functions worked as expected. However, due to AI's difficulty in
understanding long specifications like VirtIO, the tests were often incorrect.
The coding agent was also unfamiliar with using the `dd` CLI tool to create test
disk images, resulting in partition tables being misaligned and our FAT driver
hanging indefinitely when trying to read from them. After 5 hours of prompting
(which maxed out Kedar's monthly Cursor credits, Antigravity credits for **ALL
FIVE MODELS**, used 10% of his monthly Perplexity Deep Research queries, and
consumed 12% of his Copilot Enterprise premium quota for the month), we finally
reached a semblance of a working test suite. Even then, many tweaks needed to be
made to the test code's Makefile to get it to build.

### Testing Process
Because of the unique challenges posed by our AI-generated test suite, our
testing approach was a mix of formal, automated tests and informal, manual validation.
The AI-generated test suite was intended to be a formal set of automated tests
that checked each function exposed by the driver libraries we developed.
Thankfully, since the UART driver worked from the get-go, we were able to use it
to print debug information during testing.

However, as described above, the AI-generated tests were buggy themselves. This
resulted in us manually testing our tests by prompting the AI to check whether
the test code was correct. Since most of the bugs resulted in our drivers
hanging indefinitely, the AI models we prompted couldn't handle running shell
commands autonomously as frequent human intervention was required to kill
hanged terminals (where even Ctrl+C didn't exit) this was a slow and arduous
process, and as detailed above, was detrimental to my monthly AI usage quotas.
