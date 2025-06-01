# Parallel Text Processing System

A high-performance multithreaded text analysis system implemented in C++ using pthreads for parallel processing of large text datasets. The system processes UN document XML files to perform comprehensive word frequency analysis and linguistic statistics.

## Features

- **Multithreaded Processing**: Utilizes 8 worker threads for parallel text processing
- **CPU Affinity Optimization**: Enhanced versions with CPU core pinning for maximum performance
- **Memory Efficient**: Processes text in 4KB chunks to handle large files without excessive memory usage
- **Comprehensive Analysis**: 
  - Total word count statistics
  - Unique word identification
  - Word frequency analysis with sorting
  - Vowel-starting word detection
- **XML Processing Pipeline**: Complete workflow from raw XML to clean text analysis

## Requirements

### System Requirements
- Linux/Unix system (for CPU affinity features)
- GCC compiler with C++11 support
- pthread library
- Python 3.x (for preprocessing scripts)

### Dependencies
```bash
# C++ compilation requirements
sudo apt-get install build-essential
sudo apt-get install libpthread-stubs0-dev

# Python requirements (no additional packages needed)
python3 --version
```

## Installation & Setup

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/repo.git
cd parallel-text-processing
```

2. **Prepare your dataset**
   - Place UN XML files in the directory structure: `un/xml/en/y20XX/`
   - Update paths in `Script1.py` as needed

3. **Compile the C++ programs**
```bash
# Basic versions
g++ -std=c++11 -pthread -O2 program.cpp -o word_analyzer
g++ -std=c++11 -pthread -O2 program.cpp -o frequency_analyzer

# CPU affinity optimized versions
g++ -std=c++11 -pthread -O2 -D_GNU_SOURCE program.cpp -o word_analyzer_affinity
g++ -std=c++11 -pthread -O2 program.cpp -o frequency_analyzer_affinity
```

## Usage Workflow

### Step 1: Extract Text from XML Files
```bash
python3 Script1.py
```
This extracts all text content from XML files and creates `text.txt`.

### Step 2: Clean XML Tags
```bash
python3 Script2.py
```
This removes XML tags and creates `clean_text.txt`.

### Step 3: Run Text Analysis

**Basic Word Analysis:**
```bash
./word_analyzer
```

**Enhanced Frequency Analysis:**
```bash
./frequency_analyzer
```

**CPU Affinity Optimized Versions:**
```bash
./word_analyzer_affinity
./frequency_analyzer_affinity
```

## Output

### Console Output
- Real-time processing status
- Total word count
- Unique word count
- Words starting with vowels
- Top 10 most frequent words
- Execution time

### File Outputs
- `3output.txt`: Complete word frequency analysis (sorted by frequency)
- `3affinityoutput.txt`: Output from CPU affinity optimized version

## Configuration

### Thread Configuration
Modify the thread count in the source files:
```cpp
#define numberofthreads 8  // Adjust based on your CPU cores
```

### Chunk Size Optimization
Adjust processing chunk size:
```cpp
#define sizeofchunk 4096  // Modify for memory/performance balance
```

### File Paths
Update input/output paths in Python scripts:
```python
input_directory = r"path/to/your/xml/files"
output_file = r"output/text.txt"
```

## Performance Features

### CPU Affinity Optimization
The affinity versions include:
- Thread pinning to specific CPU cores
- Reduced context switching overhead
- Better cache locality
- Optimal resource utilization

### Memory Management
- Efficient string handling with move semantics
- Reserved buffer allocation
- Thread-local data structures
- Minimal memory footprint per thread

### Synchronization
- Mutex-protected task queue
- Condition variable signaling
- Lock-free local processing
- Efficient thread coordination

## Performance Testing

Compare performance between versions:
```bash
# Test basic version
time ./word_analyzer

# Test optimized version  
time ./word_analyzer_affinity

# Monitor CPU usage
htop
```

## Algorithm Details

### Text Processing Pipeline
1. **File Reading**: Sequential chunk-based file reading
2. **Task Distribution**: Producer-consumer pattern with thread-safe queue
3. **Parallel Processing**: Each thread processes assigned text chunks
4. **Word Extraction**: Character-by-character parsing with case normalization
5. **Local Aggregation**: Thread-local frequency maps
6. **Global Merging**: Final consolidation of results
7. **Sorting & Output**: Frequency-based sorting and file generation

### Thread Synchronization
- **Mutex Protection**: Shared queue access control
- **Condition Variables**: Efficient thread wake-up mechanism
- **Atomic Operations**: Safe flag checking and updates

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/improvement`)
3. Commit changes (`git commit -am 'Add new feature'`)
4. Push to branch (`git push origin feature/improvement`)
5. Create Pull Request

## Acknowledgments

- UN Parallel Corpus for test data
- pthread library for multithreading support
- Linux kernel for CPU affinity capabilities
