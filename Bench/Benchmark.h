//+------------------------------------------------------------------+
//|              Bench - a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include "TestFactory.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <barrier>

//+------------------------------------------------------------------+
//| Configuration of a single test                                   |
//+------------------------------------------------------------------+
struct TestCfg {
   struct ThreadInit {
      std::string initializer;
      std::string context;
   };

   typedef std::vector<ThreadInit>                      Threads;
   typedef std::unordered_map<std::string, std::string> Contexts;
   

   std::string    name;                      // test name
   std::string    library;                   // name of the DLL
   size_t         concurrency;               // number of concurrent threads
   size_t         samples;                   // number of test iterations per thread
   ThreadInit     thread_default;            // default threads initializer
   Threads        threads;                   // per-thread initializers
   Contexts       contexts;                  // contexts map [name=>initializer]
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
   std::string    context_init;
   ITest*         instance;
   size_t         samples;
   Timings        timings;
};
//+------------------------------------------------------------------+
//| Per-thread statistics                                            |
//+------------------------------------------------------------------+
struct RunThreadStats {
   uint64_t       min;
   uint64_t       max;
   uint64_t       avg;
   uint64_t       sum;
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
   void              ProcessTimings(size_t id, RunTestCfg* test, RunThreadStats& stats);
};
//+------------------------------------------------------------------+
