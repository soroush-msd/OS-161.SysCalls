# OS/161 Project: File System Calls Implementation

## Introduction

This repository contains the implementation of a software bridge between the file-related system calls inside the OS/161 kernel and their functionality within the VFS. Upon completion, the OS/161 system will support running a single user-level application with basic file I/O operations.

The primary focus of this project is to:
1. Understand the OS/161 kernel architecture.
2. Implement the required file-based system calls: `open`, `read`, `write`, `lseek`, `close`, and `dup2`.
3. Design robust system call interfaces and structures.

Optionally, an advanced implementation includes process-related system calls and the ability to run multiple applications.

---

## Goals

1. **Basic Project:**
   - Transform OS/161 into an operating system capable of running compiled user-level programs.
   - Implement part of the interface between user-mode programs and the kernel.
   - Provide support for system calls related to file manipulation.
   
2. **Advanced Project (Optional):**
   - Add process-related system calls.
   - Allow concurrent execution of multiple processes.

---

## Features Implemented

### 1. File-Based System Calls
The following system calls are implemented:
- `open()`
- `read()`
- `write()`
- `lseek()`
- `close()`
- `dup2()`

### 2. Robust Error Handling
All system calls are designed to gracefully handle error conditions as per the OS/161 man pages, ensuring the system remains stable under various scenarios.

### 3. Standard File Descriptors
- `stdin` (0) can remain unattached.
- `stdout` (1) and `stderr` (2) are attached to the console device (`con:`).
- Programs can use `dup2()` to change these descriptors.

---

## User-Level Programs

The `System/161` simulator can execute user-level C programs compiled using `os161-gcc`, producing MIPS executables. A variety of pre-existing programs are available in the following directories:
- `userland/bin`
- `userland/testbin`
- `userland/sbin`

New programs for testing purposes can be added by modifying the `Makefile` and creating directories similar to the existing ones.

---

## Design Approach

### Design Philosophy
1. **Plan Before Coding**: A robust design was developed before implementation, focusing on understanding the problem and determining the relationship between components.
2. **Focus on State Management**: System calls are primarily about managing file descriptors and filesystem state.
3. **Scalability**: While the basic project assumes single-process execution, the implementation was designed to support future multi-process capabilities.

### Key Design Decisions
- **File Descriptor Management**: Each process maintains its own file descriptor table. Shared state (e.g., open file table) is designed to be concurrency-safe for future multi-process extensions.
- **Kernel/User Space Interface**: System calls were designed to handle erroneous arguments and prevent user programs from crashing the kernel.

---

## Implementation Details

### System Call Interface
- Kernel function prototypes are defined in `kern/include/syscall.h` or `kern/include/file.h`.
- Implementations are located in `kern/syscall/file.c`.

### Leveraging VFS
The implementation utilizes the existing VFS layer for low-level file operations, focusing on designing and managing higher-level abstractions such as file descriptors and process-specific state.

---

## Notes for Developers

1. **Error Handling**: Ensure that your syscalls return the correct values or error codes as specified in the OS/161 man pages.
2. **Standard Input/Output**: Modify `runprogram()` to initialize `stdout` and `stderr` correctly.
3. **Memory Management**: Your implementation must be memory-leak-free.

---

## Additional Resources

### Suggested Readings
- [Modern Operating Systems](https://www.pearson.com/store/p/modern-operating-systems/P100000588956)
- [The Design and Implementation of the 4.4 BSD Operating System](https://books.google.com)
- [Unix Internals: The New Frontiers](https://www.goodreads.com/book/show/125314.Unix_Internals)
