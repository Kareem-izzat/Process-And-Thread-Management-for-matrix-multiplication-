# Process and Thread Management

## Objective
The primary objective is to compare the performance of different approaches to matrix multiplication, including a naive serial solution, a process-based solution, and various thread-based solutions. The focus is on achieving optimal execution time while maintaining correct results.

Matrix multiplication is a fundamental operation in linear algebra with applications in various domains such as computer graphics, scientific computing, and machine learning. This task explores different approaches to optimize matrix multiplication in C, ranging from a naive serial implementation to parallel implementations using processes and threads.

## Implemented Approaches

### 1. Naive Approach
The naive approach represents a conventional method of matrix multiplication using nested loops. It serves as a baseline for performance comparison.

### 2. Process-Based Approach
The process-based approach involves dividing the matrix multiplication task among multiple processes. Each process is responsible for a subset of the rows, and communication between processes is achieved using inter-process communication (IPC) via pipes.

### 3. Thread-Based Approaches

#### 3.1 Join Threads
This approach uses multiple joinable threads to parallelize the matrix multiplication task. Each thread is assigned a subset of rows to compute, and results are combined upon thread completion.

#### 3.2 Mix of Join and Detached Threads
This approach combines joinable and detached threads to exploit the benefits of both. The detached threads focus on completing their tasks independently, while the joinable threads ensure synchronization before completing the entire computation.

#### 3.3 Detached Threads
This approach employs detached threads, allowing them to run independently without the need for explicit synchronization. The results are checked for correctness after completion.
#### the reasults are detailed in the attached report
