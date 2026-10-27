# xv6 Kernel Modifications: Page Tables, Lazy Allocation, and Page Replacement

This repository contains enhancements and extensions implemented in the **xv6** operating system. The work covers process address space introspection (`vmprint`), on-demand lazy memory allocation via page fault handling, and a comparative simulation of FIFO and LRU page replacement algorithms.

---

## Summary of Changes

### 1. Process Page Table Introspection (`vmprint`)
* **`vm.c`:** Implemented the kernel-level `vmprint()` function to traverse the active process page directory (`myproc()->pgdir`). It inspects valid page directory entries (PDEs) and page table entries (PTEs) below `KERNBASE`, extracting physical frame addresses and permission flags (`P`, `W`, `U`).
* **System Call Integration:** Registered system call `SYS_vmprint` (`22`) across `syscall.h`, `syscall.c`, `sysproc.c`, `user.h`, and `usys.S`.
* **Test Program (`pttest.c`):** Validates address space mappings at startup and after expanding the heap via `sbrk(4 * 4096)`.

### 2. Lazy Allocation & Page Fault Handling
* **`proc.h`:** Added a `numfaults` counter field to `struct proc`.
* **`sysproc.c`:** Modified `sys_sbrk()` to increase `myproc()->sz` without eagerly allocating physical memory when $n > 0$.
* **`vm.c`:** Updated `copyuvm()` to safely skip unallocated/unfaulted pages below `sz` during process duplication (`fork`).
* **`trap.c`:** Added a `T_PGFLT` trap handler that reads faulting addresses using `rcr2()`. Valid addresses below `sz` dynamically allocate a zeroed physical page (`kalloc`), map it (`mappages`), increment `numfaults`, and resume execution. Invalid accesses terminate only the offending process.
* **System Call Integration (`getfaults`):** Registered system call `SYS_getfaults` (`23`) to expose the fault counter.
* **Test Program (`faulttest.c`):** Verifies that writing to 5 distinct unallocated pages triggers exactly 5 page faults, and tests out-of-bounds protection.

### 3. Page Replacement Simulator (`pagerepl`)
* **`pagerepl.c`:** Implemented a user-space simulator evaluating **FIFO** and **LRU** page replacement policies against the reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`.
* **Output Analysis:** Tracks hit/fault traces per reference and computes comparative performance across 3, 4, and 5 frames.

---

## Experimental Results & Output Analysis

### Question 1: `pttest` Output
Running `pttest` inside xv6 demonstrates dynamic heap expansion:
* **Initial State:** Displays 3 valid user pages (text/data, guard page, and stack) and 2 page-table pages.
* **After `sbrk(4 * 4096)`:** Exactly 4 new contiguous virtual pages (`va 0x3000` through `va 0x6000`) appear, increasing total valid user pages from 3 to **7**.

### Question 2: `faulttest` Output
Running `faulttest` confirms lazy loading and process isolation:
```text
--- faulttest: starting ---
Initial page faults: 2
Accessed 5 pages. Total page faults: 7 (Expected: 5)
Testing invalid access above sbrk size (should kill process)...
pid 3 faulttest: access violation / invalid page fault at va 0x24000 (sz 0x23000)

### 3. Page Replacement Simulator (`pagerepl.c`)
#### Detailed Output Log (3 Frames)
```text
=== FIFO Algorithm (3 Frames) ===
Ref: 7 -> Fault   | Frames: [ 7 - - ]
Ref: 0 -> Fault   | Frames: [ 7 0 - ]
Ref: 1 -> Fault   | Frames: [ 7 0 1 ]
Ref: 2 -> Fault   | Frames: [ 2 0 1 ]
Ref: 0 -> Hit     | Frames: [ 2 0 1 ]
Ref: 3 -> Fault   | Frames: [ 2 3 1 ]
Ref: 0 -> Fault   | Frames: [ 2 3 0 ]
Ref: 4 -> Fault   | Frames: [ 4 3 0 ]
Ref: 2 -> Fault   | Frames: [ 4 2 0 ]
Ref: 3 -> Fault   | Frames: [ 4 2 3 ]
Ref: 0 -> Fault   | Frames: [ 0 2 3 ]
Ref: 3 -> Hit     | Frames: [ 0 2 3 ]
Ref: 2 -> Hit     | Frames: [ 0 2 3 ]
Ref: 1 -> Fault   | Frames: [ 0 1 3 ]
Ref: 2 -> Fault   | Frames: [ 0 1 2 ]
Ref: 0 -> Hit     | Frames: [ 0 1 2 ]
Ref: 1 -> Hit     | Frames: [ 0 1 2 ]
Ref: 7 -> Fault   | Frames: [ 7 1 2 ]
Ref: 0 -> Fault   | Frames: [ 7 0 2 ]
Ref: 1 -> Fault   | Frames: [ 7 0 1 ]
FIFO Summary -> Faults: 15, Hits: 5, Hit Ratio: 25%

=== LRU Algorithm (3 Frames) ===
Ref: 7 -> Fault   | Frames: [ 7 - - ]
Ref: 0 -> Fault   | Frames: [ 7 0 - ]
Ref: 1 -> Fault   | Frames: [ 7 0 1 ]
Ref: 2 -> Fault   | Frames: [ 2 0 1 ]
Ref: 0 -> Hit     | Frames: [ 2 0 1 ]
Ref: 3 -> Fault   | Frames: [ 2 0 3 ]
Ref: 0 -> Hit     | Frames: [ 2 0 3 ]
Ref: 4 -> Fault   | Frames: [ 4 0 3 ]
Ref: 2 -> Fault   | Frames: [ 4 0 2 ]
Ref: 3 -> Fault   | Frames: [ 3 0 2 ]
Ref: 0 -> Hit     | Frames: [ 3 0 2 ]
Ref: 3 -> Hit     | Frames: [ 3 0 2 ]
Ref: 2 -> Hit     | Frames: [ 3 0 2 ]
Ref: 1 -> Fault   | Frames: [ 1 0 2 ]
Ref: 2 -> Hit     | Frames: [ 1 0 2 ]
Ref: 0 -> Hit     | Frames: [ 1 0 2 ]
Ref: 1 -> Hit     | Frames: [ 1 0 2 ]
Ref: 7 -> Fault   | Frames: [ 1 7 2 ]
Ref: 0 -> Fault   | Frames: [ 1 7 0 ]
Ref: 1 -> Hit     | Frames: [ 1 7 0 ]
LRU Summary  -> Faults: 12, Hits: 8, Hit Ratio: 40%
