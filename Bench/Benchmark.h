//+------------------------------------------------------------------+
//|              Bench - a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include "TestFactory.h"
#include <string>
#include <vector>
#include <barrier>

//+------------------------------------------------------------------+
//| Configuration of a single test                                   |
//+------------------------------------------------------------------+
struct TestCfg {
   typedef std::vector<std::string> strings;

   std::string    name;                      // test name
   std::string    library;                   // name of the DLL
   std::string    initializer;               // global initialization string for the DLL
   size_t         concurrency;               // number of concurrent threads
   size_t         samples;                   // number of test iterations per thread
   strings        threads_initializers;      // per-thread initialization strings
};
//+------------------------------------------------------------------+
//| Configuration of a single running test thread                    |
//+------------------------------------------------------------------+
struct RunTestCfg {
   struct TimingEntry {
      uint64_t    timestamp;  // QPC timestamp
      uint64_t    duration;   // measured time of test sample
   };
   typedef std::vector<TimingEntry> Timings;

   std::string    initializer;
   ITest*         instance;
   size_t         samples;
   Timings        timings;
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class Benchmark {
   typedef std::vector<TestCfg> TConfigs;

private:
   TConfigs          m_tests;

public:
                     Benchmark();
                    ~Benchmark();

   bool              LoadConfig();
   void              Run();

private:
   void              RunTest(std::barrier<>& sync, RunTestCfg* test);
   std::string       FormatDuration(int64_t duration_ns);
   void              ProcessTimings(size_t id, RunTestCfg* test);
};
//+------------------------------------------------------------------+
