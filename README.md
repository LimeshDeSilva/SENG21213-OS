# SENG21213-OS

An educational 32-bit x86 operating system kernel built from scratch in C and Assembly for the SENG21213 Computer Architecture & Operating Systems coursework.

---

## Features & Implemented Milestones

* Stage 0: Kernel Foundations (v0.1-stage0)
  * Two-stage bootloader transitioning from 16-bit Real Mode to 32-bit Protected Mode.
  * VGA text-mode driver (80x25) with color attributes.
  * PS/2 keyboard driver with circular buffer and interactive kernel shell (ksh).

* Stage 1: Process Management (v0.2-stage1)
  * Process Control Block (PCB) tracking PID, state, and allocated stack.
  * Assembly context switching (boot/switch.asm) preserving callee/caller registers.
  * Cooperative Round-Robin scheduler (ps, spawn, yield).

* Stage 2: Kernel Threads & Synchronization (v0.3-stage2)
  * Thread Control Block (TCB) for lightweight kernel threads.
  * Hardware-level atomic mutexes utilizing lock xchgl to avoid race conditions (threads, mutex).

* Stage 3: Physical Memory Management (v0.4-stage3)
  * Bitmap page frame allocator managing 32 MB RAM (8,192 pages of 4 KB).
  * Low-memory reservation (first 1 MB reserved for BIOS/VGA/kernel).
  * Memory diagnostics and runtime frame allocation (free, alloc, dealloc).

* Stage 4: Storage & File System (v0.5-stage4)
  * In-memory RAM disk file system supporting directory tables and file metadata.
  * Interactive file operations (ls, cat, write).

---

## Toolchain & Dependencies

* gcc / gcc-multilib (32-bit target -m32)
* nasm (Netwide Assembler)
* ld / GNU Binutils (elf_i386)
* qemu-system-i386

Installation on Ubuntu/Debian:
sudo apt update && sudo apt install -y gcc gcc-multilib nasm qemu-system-x86

---

## Build & Run

* Build floppy image: make
* Run inside QEMU: make run
* Clean build artifacts: make clean

---

## Shell Commands (ksh)

* help - Displays available shell commands
* clear - Clears the VGA text screen
* about - Displays OS build metadata
* ps - Lists active processes in the process table
* spawn - Instantiates a cooperative background worker task
* yield - Relinquishes CPU slice to the next ready task
* threads - Displays active kernel threads
* mutex - Executes the atomic mutex contention test
* free - Displays physical page frame statistics
* alloc - Allocates a 4 KB physical page
* dealloc - Frees the currently allocated page frame
* ls - Lists files stored in the RAM disk
* cat <file> - Reads and displays file content
* write <file> <text> - Creates or overwrites a file with text data
