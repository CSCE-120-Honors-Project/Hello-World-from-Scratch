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

For those who really want a link to the code, here it is: https://github.com/CSCE-120-Honors-Project/DIY-Bootloader
