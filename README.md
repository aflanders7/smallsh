This repository contains a collection of C programming projects completed as part of my coursework. These projects focus on systems programming concepts including process management, file I/O, networking, encryption, and multithreading.

## Projects

### Smallsh - Custom Shell

A Unix shell implemented in C that supports running commands, managing processes, and handling signals.

**Features:**
- Execute built-in and external commands
- Support for:
  - Command line arguments
  - Input/output redirection
  - Background processes
- Process status tracking
- Signal handling for `SIGINT` and `SIGTSTP`
- Foreground/background job management

**Concepts Practiced:**
- Process creation with `fork()`
- Program execution with `exec()`
- Process synchronization with `waitpid()`
- File descriptors and redirection
- Signal handling

---

### OTP — One-Time Pad Encryption

A client/server encryption system implementing a one-time pad encryption algorithm.

**Features:**
- Encrypt plaintext messages using a randomly generated key
- Decrypt ciphertext messages
- Client/server communication over sockets
- Validation of input and key requirements
- Concurrent request handling

**Concepts Practiced:**
- Socket programming
- Client/server architecture
- Data transmission
- Encryption algorithms
- Process communication

---

### MTP — Multi-Threaded Programming Project

A C project focused on concurrency and synchronization using multiple threads.

**Features:**
- Create and manage multiple threads
- Perform concurrent operations
- Coordinate shared resources safely
- Prevent race conditions using synchronization techniques

**Concepts Practiced:**
- POSIX threads (`pthread`)
- Thread synchronization
- Mutex locks
- Race condition prevention
- Concurrent programming

---

## Technologies Used

- **Language:** C
- **Operating System Concepts:** Linux/Unix systems programming
- **Libraries & Tools:**
  - POSIX API
  - pthreads
  - Unix sockets
  - GCC
  - Makefiles
  - Git

---

## Learning Objectives

These projects provided hands-on experience with low-level software development, including:

- Managing processes and system resources
- Working with memory and file operations
- Implementing network communication
- Understanding encryption fundamentals
- Designing thread-safe applications
- Debugging C programs using Linux development tools

---

## Author

**Audrey Flanders**
