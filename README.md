
# **File Management Simulation**

## **Overview**

This project implements a multi-process simulation for handling CSV file generation, processing, and management using **Inter-Process Communication (IPC)** techniques in Linux. The system is designed to efficiently generate, process, move, and inspect files while tracking various metrics in shared memory.

### **Key Features**
- **File Generation**: Creates CSV files with user-defined characteristics (rows, columns, ranges, and missing data percentages).
- **File Processing**: Calculates column averages, excluding missing values.
- **File Movement**: Moves files to appropriate directories (Processed, UnProcessed, Backup).
- **Inspection**: Manages files based on user-defined aging policies, including deletion.
- **Threshold-Based Simulation Termination**: Ends simulation based on various user-defined thresholds.
- **Shared Metrics**: Reports key metrics like processed files, unprocessed files, averages, and more.

---

## demo
[Watch the demo video](https://github.com/user-attachments/assets/0f9f7494-fc5c-4149-9075-a20a2ba2951a)

## **How the System Works**

### **1. CSV File Generation**
- **Purpose**: Simulates data generation with randomness in file properties.
- **Configuration**:
  - User-defined number of generators (default: 5).
  - Randomized row and column counts (default: 10,000 rows and 10 columns).
  - Decimal values within a user-defined range.
  - Missing values based on a user-defined miss-percentage.

### **2. CSV File Calculators**
- **Purpose**: Processes generated files, calculating column averages.
- **Key Functions**:
  - Skips missing values when calculating averages.
  - Reports metrics in shared memory (e.g., file averages, processed file count).

### **3. CSV File Movers**
- **Purpose**: Moves processed files to a `Processed` directory.
- **Key Features**:
  - Creates the `Processed` directory if it does not exist.
  - Reports the total number of moved files.

### **4. Inspectors**
- Three types of inspectors manage file aging and lifecycle:
  - **Type 1**: Moves unhandled files older than a threshold to `UnProcessed`.
  - **Type 2**: Moves processed files older than a threshold to `Backup`.
  - **Type 3**: Deletes files in `Backup` older than a threshold.

### **5. Metrics and Shared Memory**
- Metrics include:
  - Minimum and maximum averages per column with file details.
  - Counts of processed, unprocessed, moved, and deleted files.
- Shared memory is used to synchronize data between processes.

### **6. Simulation Termination**
The simulation ends when any of the following conditions are met:
- Processed file count exceeds a threshold.
- Unprocessed file count exceeds a threshold.
- Backup file count exceeds a threshold.
- Deleted file count exceeds a threshold.
- Simulation runs for more than a user-defined duration.

---

## Solution Explanation

1. **CSV File Generation** (Generator Process)
   - Loads configuration details.
   - Creates a semaphore and a CSV file with user-defined parameters (rows, columns, ranges, miss percentage).
   - Generates random decimal values within a specified range.
   - Introduces missing values based on the miss percentage.
   - Sends the generated file name through a message queue to the calculator process.
   - Notifies inspector 1 of the file name and generation time via another message queue.

2. **CSV File Calculators** (Calculator Process)
   - Loads configuration settings.
   - Waits for a message containing the generated file name.
   - Acquires the semaphore associated with the file to ensure exclusive access.
   - Processes the file, calculating column averages while skipping missing values.
   - Reports metrics like averages and processed file count in shared memory.
   - Sends the successfully processed file number to the mover process.

3. **CSV File Movers** (Mover Process)
   - Receives the file number from the calculator process.
   - Moves the processed file to the designated "Processed" directory.
   - Creates the "Processed" directory if it doesn't exist.
   - Updates and reports the total number of moved files.
   - Sends the moved file number and time of movement to inspector 2 via a message queue.

4. **Inspectors** (Inspector 1, 2, and 3 Processes)
   - **Inspector 1:**
      - Receives generated file name and creation time from the generator.
      - Stores this information in a min-heap data structure.
      - Periodically checks the min-heap's root node.
      - If the creation time exceeds the unprocessed file threshold, moves the file to the "UnProcessed" directory.
   - **Inspector 2:**
      - Receives file number and time of move from the mover.
      - Stores this information in a min-heap.
      - Regularly checks the min-heap's root node.
      - If the time of move exceeds the processed file threshold, moves the file to the "Backup" directory.
      - Informs inspector 3 about the moved file and time of movement.
   - **Inspector 3:**
      - Receives information from inspector 2 (moved file and time).
      - Stores it in a min-heap for tracking.
      - Continuously monitors the min-heap's root node.
      - When the time exceeds the backup file threshold, deletes the file. 


## **How to Run the Program**

### **1. Prerequisites**
- A Linux machine with:
  - GCC compiler.
  - OpenGL library for visualizations.






### **2. Compilation**

To compile the project, use the `Makefile` provided. The build process supports both silent and verbose modes for flexibility during development and runtime.

#### **Silent Compilation (Default)**
The default behavior compiles the project without enabling verbose output (minimal logging):
```bash
make
```
- This mode is ideal for production or when you don't need detailed logs.
- The program runs quietly, focusing on functionality and performance.

#### **Verbose Compilation**
To enable detailed print statements for debugging or monitoring during runtime, use the `__CLI` flag:
```bash
make __CLI=1
```
- This mode is useful for development or troubleshooting.
- Detailed logs and progress messages will be printed to the CLI during execution.

#### **Clean Build**
To remove the compiled binary and perform a fresh build:
```bash
make clean
```
- Ensures no residual artifacts from previous builds interfere with the current build.




### **3. Execution**
Run the main program with a configuration file:
```bash
./bin/file-managment-simulation configuration.txt
```


**Additional Notes**

- The `configuration.txt` file should contain the necessary parameters for the simulation, such as the number of generators, file size limits, and threshold values.
- The project utilizes IPC mechanisms like message queues and shared memory to enable efficient communication and data sharing between processes.
- The visualization component (using OpenGL) provides a visual representation of the system's state and performance metrics.


