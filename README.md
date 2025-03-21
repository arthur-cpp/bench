# Bench - A Benchmark Tool for Plugin-Based Performance Testing

## Overview
Bench is a powerful benchmark tool designed to run performance tests as plugins. It provides a flexible framework for executing concurrent performance tests with configurable parameters and detailed timing measurements.

## Features
- Plugin-based architecture for easy test implementation
- Support for concurrent test execution with multiple threads
- Configurable test parameters including:
  * Number of concurrent threads
  * Number of test iterations per thread
  * Global and per-thread initialization
- High-precision timing measurements
- Detailed timing analysis and reporting
- Memory leak detection in debug mode

## Project Structure
- `Bench/` - Main project directory containing the core benchmark engine
  * `Benchmark.h/cpp` - Core benchmark implementation
  * `TestFactory.h/cpp` - Plugin management and test instantiation
  * `Bench.cpp` - Main entry point
- `BenchPluginEmpty/` - Template project for creating new test plugins

## Building
The project uses Visual Studio 2022 and requires **yaml-cpp** library. The library should be installed using **vcpkg**:

- install **vcpkg** to the `C:\local\vcpkg\`
- then install **yaml-cpp**: `vcpkg install yaml-cpp:x64-windows-static`
- the project assumes the following directory structure:
  * include: `C:\local\vcpkg\installed\x64-windows-static\include`
  * library: `C:\local\vcpkg\installed\x64-windows-static\lib` (for **Debug**: `C:\local\vcpkg\installed\x64-windows-static\debug\lib`)
- linker uses `yaml-cppd.lib` for **Debug** and `yaml-cpp.lib` for **Release** versions


## Usage
1. Build the main Bench project
2. Create test plugins using the `BenchPluginEmpty` template
3. Copy the compiled plugin DLLs to the `tests` folder located next to the main executable
4. Configure test parameters in the `config.yaml` configuration file
5. Run the benchmark tool

The directory structure is as follows:

```plaintext
bench.exe
config.yaml
tests/
    empty.dll
```

## Configuration
The benchmark tool uses a YAML configuration file to specify test parameters. Here's a sample `config.yaml`:

```yaml
concurrency: 16  # number of concurrent threads
samples: 1000000 # number of test iterations per thread

tests:
  - name: Test
    load: empty.dll
    init: "Test"
    samples: 10000
    concurrency: 8
    threads:
      - "ThreadInitA"
      - "ThreadInitB"
      - "ThreadInitC"

  - name: "Another Test"
    load: AnotherTestPlugin.dll
    concurrency: 2
    samples: 500
```

Configuration parameters:

### Configuration parameters:
- `concurrency`: number of concurrent threads to run the test
- `samples`: number of test iterations per thread
- `tests`: a list of tests with their parameters
- `name`: unique identifier for the test
- `load`: DLL file containing the test implementation
- `init`: initialization string passed to the test
- `threads`: list of initialization strings for each thread, used with revolver principe

## License

MIT.

## Author
Arthur Valitov
https://github.com/arthur-cpp

Copyright (c) 2025, Arthur Valitov
