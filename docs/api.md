# Proxima API Reference

## Version 1.0.0

## Table of Contents

1. [Core Types](#core-types)
2. [Standard Library](#standard-library)
3. [Compiler API](#compiler-api)
4. [IDE API](#ide-api)
5. [Runtime API](#runtime-api)

## Core Types

### Scalar Types

| Type | Size | Description |
| ---- | ---- | ----------- |
| `int4` | 4-bit | Signed integer |
| `int8` | 8-bit | Signed integer |
| `int16` | 16-bit | Signed integer |
| `int32` | 32-bit | Signed integer (default `int`) |
| `int64` | 64-bit | Signed integer |
| `single` | 32-bit | IEEE 754 float |
| `double` | 64-bit | IEEE 754 double |
| `bool` | 1-bit | Boolean (true/false) |
| `char` | 8-bit | Character |
| `string` | variable | String of characters |
| `time` | 64-bit | Time timestamp |

### Composite Types

| Type | Description |
| -----| ----------- |
| `vector<T>` | One-dimensional array |
| `matrix<T>` | Two-dimensional array |
| `layer<T>` | Three-dimensional array |
| `collection` | Table with headers |
| `point2<T>` | 2D point |
| `point3<T>` | 3D point |
| `point4<T>` | 4D point |

### Special Types

| Type | Description |
| ---- | ----------- |
| `auto` | Automatic type inference |
| `void` | No return value |
| `type` | Type information |
| `method` | Function/method pointer |
| `rtti` | Runtime type information |
| `null` | Null pointer |

## Standard Library

### I/O Functions

``` proxima
// Console output
print(format: string, ...args) : void
print(file: file, format: string, ...args) : void

// File operations
file.open(path: string, mode: string) : bool
file.close() : void
file.read() : string
file.write(data: string) : void
file.seek(position: int64) : void
file.exists(path: string) : bool

// Serialization
write(file: file, data: auto) : void
read(file: file, type: type) : auto
```

### Math Functions

``` proxima
// Basic math
abs(x: auto) : auto
sqrt(x: auto) : auto
exp(x: auto) : auto
log(x: auto) : auto
log10(x: auto) : auto

// Trigonometric
sin(x: auto) : auto
cos(x: auto) : auto
tan(x: auto) : auto
asin(x: auto) : auto
acos(x: auto) : auto
atan(x: auto) : auto

// Matrix operations
zeros(rows: int, cols: int) : matrix<double>
ones(rows: int, cols: int) : matrix<double>
eye(size: int) : matrix<double>
rand(rows: int, cols: int) : matrix<double>
size(m: matrix) : [rows: int, cols: int]
transpose(m: matrix) : matrix
```

### Collection Functions

``` proxima
// Creation
collection() : collection
collection(headers: vector<string>) : collection

// Operations
addRow(c: collection, values: vector<auto>) : void
removeRow(c: collection, index: int) : void
getColumn(c: collection, name: string) : vector<auto>
setColumn(c: collection, name: string, values: vector<auto>) : void

// File I/O
collection.read(path: string) : collection
collection.write(path: string) : void
```

### Time Functions

``` proxima
// Creation
time() : time
time(timestamp: int64) : time
time(formatted: string, format: string) : time

// Operations
time.formatted() : string
time.add(seconds: int64) : time
time.subtract(other: time) : int64

// Current time
time_now() : time
```

### Debug Functions

``` proxima
// Debug control
dbgstop() : void
dbgprint(...args, level: int) : void
dbgcontext() : void
dbgstack() : void

// Breakpoints
breakpoint() : void
breakpoint(count: int) : void
```

### Parallel Functions

``` proxima
// GPU configuration
parallel(grid: int, block: int) : parallel
parallel.set(host_var: auto) : auto
parallel.get(gpu_var: auto) : auto

// CPU parallel
parallel(threads: int, array: auto*) for i in range
    // parallel code
end
```

## Compiler API

### Command Line

``` bash
#Build
proxima build [options] [files...]

#Run
proxima run [options] [files...]

#Test
proxima test [suite] [test]

#Time
proxima time [suite] [test]

#Documentation
proxima doc [options]

#Dependencies
proxima deps

#Clean
proxima clean
```

### Options


| Option | Description |
| ------ | ----------- |
| -o, --output | Output file |
| -O0, -O1, -O2, -O3 | Optimization level |
| -v, --verbose | Verbose level (0-5) |
| --debug | Enable debug symbols |
| --release | Release mode |
| --cuda | Enable CUDA |
| --avx2 | Enable AVX2 |
| --max-memory | emory limit |

## IDE Protocol

``` proxima
// Request from IDE
[
    "request", "analyze_file",,
    "file", "/path/to/module.prx",,
    "options", [
        "infer_types", true,,
        "highlight_timing", true
    ]
]

// Response from compiler
[
    "status", "ok",,
    "symbols", [...],,
    "warnings", [...],,
    "timing_hints", [...]
]
```

## IDE API

### MainWindow

``` proxima
// Project management
bool newProject(path: string, name: string)
bool openProject(path: string)
bool saveProject()
bool closeProject()

// File management
bool openFile(path: string)
bool saveFile(path: string)
bool saveAllFiles()
void closeFile(path: string)

// Execution
void runProgram(mode: RunMode)
void pauseProgram()
void stopProgram()
void stepOver()
void stepInto()
void stepOut()
void continueExecution()

// Debugging
void addBreakpoint(file: string, line: int, type: BreakpointType)
void removeBreakpoint(id: int)
void toggleBreakpoint(file: string, line: int)
```

### Code Editor

``` proxima
// File operations
bool loadFile(path: string)
bool saveFile()
string getFilePath()
bool isModified()

// Cursor
int getCurrentLine()
int getCurrentColumn()
void goToLine(line: int)

// Display modes
void setDisplayMode(mode: DisplayMode)
void applyTypeHighlighting(types: map<int, string>)
void applyTimingHighlighting(timings: map<int, double>)

// Breakpoints
void toggleBreakpoint(line: int)
bool hasBreakpoint(line: int)
vector<int> getBreakpoints()
```

### Visualizers

``` proxima
// Vector plot
void setData(data: vector<double>)
void zoomIn()
void zoomOut()
void findPeaks()
void findValleys()
void copyData()
void saveAsCSV(path: string)

// Matrix view
void setData(data: matrix<double>)
void setViewMode(mode: MatrixViewMode)
void setColorPalette(palette: ColorPalette)
void selectCell(row: int, col: int)
void copyData()

// Layer 3D view
void setData(data: layer<double>)
void setCurrentSlice(slice: int)
void setViewMode(mode: int)
void setRotation(x: double, y: double, z: double)
void saveSliceAsImage(path: string, slice: int)

// Collection table
void setData(data: vector<vector<string>>, headers: vector<string>)
void sort(column: int, order: SortOrder)
void setFilter(column: int, filter: string)
void copyToClipboard()
void saveAsCSV(path: string)
```

## Runtime API

### Memory Management

``` proxima
// Memory info
memory_free() : int64
memory_total() : int64
cpu_usage() : single
disk_free() : int64

// Allocation
allocate(size: size_t) : void*
deallocate(ptr: void*) : void
```

### System Functions

``` proxima
// Time
time_now() : time
sleep(milliseconds: int) : void

// Process
exit(code: int) : void
getpid() : int
```

### GPU Functions

``` proxima
// CUDA
cuda_available() : bool
cuda_device_count() : int
cuda_memory_info() : [free: int64, total: int64]

// Memory transfer
cuda_memcpy_host_to_device(dst: void*, src: void*, size: size_t) : void
cuda_memcpy_device_to_host(dst: void*, src: void*, size: size_t) : void
cuda_malloc(size: size_t) : void*
cuda_free(ptr: void*) : void
```

## Testing Framework

### Test Suite

``` proxima
suite matrix_tests
    test test_mult_id() : void
        I: matrix33 = eye(3);
        A: matrix33 = rand(3, 3);
        assert(A * I == A, "Identity multiplication failed");
    end

    test test_gpu_same() : void
        data: matrix<double> = rand(100, 100);
        cpu_result = compute_cpu(data);
        gpu_result = compute_gpu(data);
        assert(max(abs(cpu_result - gpu_result)) < 1e-10);
    end
end
```

### Assertions

``` proxima
assert(condition: bool, message: string) : void
assert_eq(expected: auto, actual: auto, message: string) : void
assert_near(expected: double, actual: double, tol: double, message: string) : void
assert_null(ptr: auto, message: string) : void
assert_not_null(ptr: auto, message: string) : void
fail(message: string) : void
skip(message: string) : void
```

## Documentation Generation

### Doc Comments

``` proxima
// @method function_name
// @param name: type - description
// @return type - description
// @option name: type - description
// @example code example
// @see related_function
```

### Generation Commands


``` bash
# Generate all documentation
proxima doc

# HTML format
proxima doc --format=html

# PDF format
proxima doc --format=pdf

# Markdown format
proxima doc --format=md

# Include tests
proxima doc --include-tests
```

## Поддержка

- Сайт: "https://prxlang.ru"
- Email: dev@prxlang.ru
- Discord: "https://discord.gg/prxlang"
- Issues: "https://github.com/snowfire-gtc/Proxima-Centauri/issues"

## Proxima Development Team © 2026
