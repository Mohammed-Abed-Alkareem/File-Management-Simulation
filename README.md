
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


//add 


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

### **4. Configuration File**
- Create a `config.txt` file to specify user-defined parameters, such as:
  - Number of generators, calculators, and inspectors.
  - Row and column counts.
  - File aging thresholds.
  - Time duration for simulation.
  - Directory paths for file management.

Example `config.txt`:
```plaintext
NUM_GENERATORS=5
NUM_CALCULATORS=3
NUM_MOVERS=10
NUM_INSPECTORS_TYPE1=2
NUM_INSPECTORS_TYPE2=2
NUM_INSPECTORS_TYPE3=2
ROW_COUNT=10000
COLUMN_COUNT=10
VALUE_RANGE_MIN=0.0
VALUE_RANGE_MAX=100.0
MISS_PERCENTAGE=5
AGING_THRESHOLD=10
MAX_PROCESSED_FILES=1000
MAX_UNPROCESSED_FILES=500
SIMULATION_DURATION=60
```

---

