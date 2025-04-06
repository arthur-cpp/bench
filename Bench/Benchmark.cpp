//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include <iostream>
#include <thread>
#include <chrono>
// yaml-cpp
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
// own headers
#include "Benchmark.h"

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Benchmark::Benchmark() {}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Benchmark::~Benchmark() {}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
bool Benchmark::LoadConfig() {
   char filename[MAX_PATH];
   try {
      _snprintf_s(filename, _countof(filename), _TRUNCATE, "%s\\config.yaml", ExtProgramPath);
      YAML::Node config = YAML::LoadFile(filename);

      // global settings
      int concurrency, samples;

      // number of concurrent threads
      if (config["concurrency"]) concurrency = config["concurrency"].as<int>();
      else                       concurrency = std::thread::hardware_concurrency();
      // number of test iterations per thread
      if (config["samples"])     samples = config["samples"].as<int>();
      else                       samples = 1'000'000;

      // read tests configurations
      for (const auto& test : config["tests"]) {
         TestCfg cfg;
         // test name and name of the DLL
         cfg.name   = test["name"].as<std::string>();
         cfg.library= test["load"].as<std::string>();
         
         // global test initialization string
         if (test["init"])
            cfg.thread_default.initializer = test["init"].as<std::string>();
         // global test context
         if (test["context_init"])
            cfg.thread_default.context = test["context_init"].as<std::string>();
         
         // number of concurrent threads
         if (test["concurrency"]) cfg.concurrency = test["concurrency"].as<int>();
         else                     cfg.concurrency = concurrency;
         // number of test iterations per thread
         if (test["samples"])     cfg.samples = test["samples"].as<int>();
         else                     cfg.samples = samples;
         // fix if there is some mistakes
         if (cfg.concurrency < 1) cfg.concurrency = 1;
         if (cfg.samples< 1)      cfg.samples     = 1;

         // per-thread initialization strings
         if (test["threads"]) {
            for (const auto& thread : test["threads"]) {
               TestCfg::ThreadInit t;
               // check and fill the fields
               if (thread["init"])    t.initializer = thread["init"].as<std::string>();
               if (thread["context"]) t.context     = thread["context"].as<std::string>();
               // add new thread config
               cfg.threads.push_back(std::move(t));
            }
         }
         // contexts map
         if (test["contexts"]) {
            for (const auto& context : test["contexts"]) {
               cfg.contexts[context["name"].as<std::string>()] = context["init"].as<std::string>();
            }
         }

         // add test to the list
         m_tests.emplace_back(std::move(cfg));
      }
   }
   catch (const std::exception& e) {
      std::cerr << "Config parsing error: " << e.what() << "\n";
      return false;
   }

   return m_tests.size() > 0;
}
//+------------------------------------------------------------------+
//| Run tests                                                        |
//+------------------------------------------------------------------+
void Benchmark::Run() {
   for (auto& cfg : m_tests) {
      TestFactory factory;
      // load library
      if (factory.Load(cfg.library.c_str(), cfg.thread_default.initializer.c_str())) {
         std::vector<RunTestCfg*>                tests;
         std::vector<std::thread>                threads;
         std::unordered_map<std::string, UINT64> contexts;
         std::barrier             sync_point(cfg.concurrency + 1);
         LARGE_INTEGER            tqpc; // test start timestamp
         // create and run test threads
         for (size_t i = 0; i < cfg.concurrency; i++) {
            bool failed = true;
            // create test instance
            RunTestCfg* test = new RunTestCfg();
            if (test) {
               // store pointer
               tests.push_back(test);
               // set default context
               test->context_init = cfg.thread_default.context;
               // select initializer for thread using revolver principe
               if (cfg.threads.size() > 0) {
                  auto& t = cfg.threads[i % cfg.threads.size()];
                  test->initializer = t.initializer;
                  // detect context
                  if (!t.context.empty()) {
                     auto cit = cfg.contexts.find(t.context);
                     if (cit != cfg.contexts.end()) {
                        test->context_init = cit->second;
                     }
                  }
               }
               // dynamically create contexts
               UINT64 ctx = 0;
               if (!test->context_init.empty()) {
                  auto cit = contexts.find(test->context_init);
                  if (cit != contexts.end()) {
                     ctx = cit->second;
                  }
                  else {
                     ctx = factory.CreateContext(test->context_init.c_str());
                     if (ctx) {
                        contexts[test->context_init] = ctx;
                     }
                  }
               }
               // store samples count and create test instance
               test->samples  = cfg.samples;
               test->instance = factory.CreateTest(test->initializer.c_str(), ctx);
               // check and run thread
               if (test->instance) {
                  // run thread
                  threads.emplace_back(&Benchmark::RunTest, this, std::ref(sync_point), test);
                  // reset failed flag
                  failed = false;
               }
            }
            // check failed
            if (failed) {
               std::cout << "Test \"" << cfg.name << "\" failed to run new thread" << std::endl;
            }
         }

         // start all threads simultaneously
         std::cout << "======================================================================================" << std::endl;
         std::cout << "Test \"" << cfg.name << "\" started: " << cfg.concurrency << " threads, " << cfg.samples << " samples" << std::endl;
         auto start_time = std::chrono::high_resolution_clock::now();
         QueryPerformanceCounter(&tqpc);
         sync_point.arrive_and_wait();
         
         // wait threads to complete
         for (auto& t : threads) t.join();

         // print the overall test time
         auto end_time = std::chrono::high_resolution_clock::now();
         auto total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
         std::cout << "Test \"" << cfg.name << "\" completed in " << FormatDuration(total_time) << ":" << std::endl << std::endl;

         // initialize overall threads stats
         RunThreadStats tstats;
         tstats.min = ULLONG_MAX;
         tstats.max = 0;
         tstats.sum = 0;
         tstats.avg = 0;

         // release contexts
         for (const auto& ctx : contexts) {
            factory.DestroyContext(ctx.second);
         }
         // release tests instances and calculate statistics
         size_t counter = 1;
         for (auto test: tests) {
            RunThreadStats stats;
            // release test instance
            test->instance->Release();
            // process timings
            ProcessTimings(counter++, test, stats);
            // update overall stats
            tstats.sum += stats.sum;
            tstats.avg += stats.avg;
            if (stats.min < tstats.min) tstats.min = stats.min;
            if (stats.max > tstats.max) tstats.max = stats.max;
            // free memory
            delete test;
         }
         // print min/min, max/max, avg/avg for all threads
         if (counter > 0) {
            tstats.avg = tstats.avg / counter;

            std::cout << "  ["
               << std::setw(2)  << std::right << "**" << "] min/max/avg = "
               << std::setw(10) << std::right << FormatDuration(tstats.min) << " / "
               << std::setw(10) << std::right << FormatDuration(tstats.max) << " / "
               << std::setw(10) << std::right << FormatDuration(tstats.avg) << " / -"
               << std::endl;
         }
         // final statistics
         std::cout << std::endl << "Test \"" << cfg.name << "\" summ run time " << FormatDuration(tstats.sum)  << std::endl;
         std::cout << "======================================================================================" << std::endl;
      }
      else {
         std::cerr << "Test \"" << cfg.name << "\" load failed" << std::endl;
      }
   }
}
//+------------------------------------------------------------------+
//| Run single instance of test                                      |
//+------------------------------------------------------------------+
void Benchmark::RunTest(std::barrier<>& sync, RunTestCfg* test) {
   // checks
   if (!test) return;
   // prepare size for timings
   test->timings.resize(test->samples);
   
   // synchronize start
   sync.arrive_and_wait();

   // run test
   if (test->instance) {
      auto instance = test->instance;
      auto timings  = test->timings.data();
      // loop through
      size_t count;
      for (count = 0; count < test->samples; count++) {
         // prepare before test
         if (!instance->RunBefore()) break;
         // record timestamp
         LARGE_INTEGER qpc;
         QueryPerformanceCounter(&qpc);

         // run one sample of the test
         auto start = std::chrono::high_resolution_clock::now();
         if (!instance->Run()) break;
         auto end = std::chrono::high_resolution_clock::now();
         
         // store timing data (timestamp and duration)
         auto t = timings + count;
         t->timestamp = qpc.QuadPart;
         t->duration  = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

         // after test
         if (!instance->RunAfter()) break;
      }
      // cut to actual samples count
      test->timings.resize(count);
   }
}
//+------------------------------------------------------------------+
//| Format duration as string                                        |
//+------------------------------------------------------------------+
std::string Benchmark::FormatDuration(int64_t duration_ns) {
   constexpr int precision = 3;

   if (duration_ns < 1'000) {                // < 1 us
      return std::to_string(duration_ns) + " ns";
   }
   else if (duration_ns < 1'000'000) {       // < 1 ms
      double us = duration_ns / 1'000.0;
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(precision) << us << " us";
      return oss.str();
   }
   else if (duration_ns < 1'000'000'000) {   // < 1 sec
      double ms = duration_ns / 1'000'000.0;
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(precision) << ms << " ms";
      return oss.str();
   }
   else {                                    // >= 1 sec
      double sec = duration_ns / 1'000'000'000.0;
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(precision) << sec << " seconds";
      return oss.str();
   }
}
//+------------------------------------------------------------------+
//| Process timings from single thread                               |
//+------------------------------------------------------------------+
void Benchmark::ProcessTimings(size_t id, RunTestCfg* test, RunThreadStats& stats) {
   // initialize stats
   stats.min = ULLONG_MAX;
   stats.max = 0;
   stats.avg = 0;
   stats.sum = 0;
   // checks
   if (!test) return;

   // calculate statistics
   if (test->timings.size() > 0) {
      
      for (auto& timing : test->timings) {
         auto& t = timing.duration;
         stats.sum += t;
         if (t < stats.min) stats.min = t;
         if (t > stats.max) stats.max = t;
      }
      stats.avg = stats.sum / test->timings.size();

      std::cout << "  ["
                << std::setw(2)  << std::right << id << "] min/max/avg = "
                << std::setw(10) << std::right << FormatDuration(stats.min) << " / "
                << std::setw(10) << std::right << FormatDuration(stats.max) << " / "
                << std::setw(10) << std::right << FormatDuration(stats.avg) << " / "
                << (test->context_init.empty() ?"":("("+ test->context_init +") "))
                << (test->initializer.empty()?"-": test->initializer)
                << std::endl;
   }
   else {
      std::cout << "  ["
                << std::setw(2)  << std::right << id << "] min/max/avg = "
                << std::setw(39) << std::right << "/ "
                << (test->context_init.empty() ? "" : ("(" + test->context_init + ") "))
                << (test->initializer.empty() ? "-" : test->initializer)
                << std::endl;
   }
}
//+------------------------------------------------------------------+
