//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "Test.h"
#include <iostream>
#include <chrono>
#include <algorithm>

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Test::Test() {}
//+------------------------------------------------------------------+
//| Cleanup                                                          |
//+------------------------------------------------------------------+
Test::~Test() {
   // release tests instances
   for (auto test : m_tests) {
      test->instance->Release();
      delete test;
   }

   // release contexts
   for (const auto& ctx : m_contexts) {
      m_factory.DestroyContext(ctx.second);
   }
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
bool Test::Initialize(const TestCfg& cfg) {
   // store test name
   m_name = cfg.name;
   // load library
   if (!m_factory.Load(cfg.library.c_str(), cfg.thread_default.initializer.c_str())) {
      std::cerr << "Test \"" << m_name << "\" load failed" << std::endl;
      return false;
   }
   
   // create and configure threads configurations
   for (size_t i = 0; i < cfg.concurrency; i++) {
      // create test instance
      RunTestCfg* test = new RunTestCfg();
      if (test) {
         // store samples count
         test->samples = cfg.samples;
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
         // create test instance
         test->instance = m_factory.CreateTest(test->initializer.c_str(), CreateContext(test->context_init));
         // store pointer on successfully created test instance
         if (test->instance) {
            m_tests.push_back(test);
         }
         else {
            std::cout << "Test \"" << m_name << "\" failed to create test "
                      << (test->context_init.empty() ? "" : ("(" + test->context_init + ") "))
                      << (test->initializer.empty() ? "-" : test->initializer)
                      << std::endl;
            // release memory
            delete test;
         }
      }
   }

   // return result
   return m_tests.size() > 0;
}
//+------------------------------------------------------------------+
//| Dynamically create reusable contexts                             |
//+------------------------------------------------------------------+
UINT64 Test::CreateContext(const std::string& context_init) {
   UINT64 ctx = 0;
   // check initialization string is not empty
   if (!context_init.empty()) {
      // try find already created context
      auto cit = m_contexts.find(context_init);
      if (cit != m_contexts.end()) {
         ctx = cit->second;
      }
      else {
         // context not created yet, create it and store to the map
         ctx = m_factory.CreateContext(context_init.c_str());
         if (ctx) {
            m_contexts[context_init] = ctx;
         }
      }
   }
   // return context id
   return ctx;
}
//+------------------------------------------------------------------+
//| Run all tests instances threads                                  |
//+------------------------------------------------------------------+
void Test::Run() {
   std::barrier   sync_point(m_tests.size() + 1);
   
   // check and run threads
   for (auto& test : m_tests) {
      m_threads.emplace_back(&Test::RunTest, this, std::ref(sync_point), test);
   }

   // print test started
   std::cout << "======================================================================================" << std::endl;
   std::cout << "Test \"" << m_name << "\" started: " << m_tests.size() << " threads" << std::endl;

   // start all threads simultaneously
   auto start_time = std::chrono::high_resolution_clock::now();
   sync_point.arrive_and_wait(); // here is all threads start
   // wait threads to complete
   for (auto& t : m_threads) t.join();
   // calculate total time
   auto end_time = std::chrono::high_resolution_clock::now();
   auto total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

   // print the overall test time
   std::cout << "Test \"" << m_name << "\" completed in " << FormatDuration(total_time) << ":" << std::endl << std::endl;

}
//+------------------------------------------------------------------+
//| Run single instance of test                                      |
//+------------------------------------------------------------------+
void Test::RunTest(std::barrier<>& sync, RunTestCfg* test) {
   // synchronize start
   sync.arrive_and_wait();

   // check pointer
   if (!test) return;
   // prepare size for timings
   test->timings.resize(test->samples);


   // run test
   if (test->instance) {
      auto instance = test->instance;
      auto timings = test->timings.data();
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
         t->duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

         // after test
         if (!instance->RunAfter()) break;
      }
      // cut to actual samples count
      test->timings.resize(count);
   }
}
//+------------------------------------------------------------------+
//| Calculate statistics                                             |
//+------------------------------------------------------------------+
void Test::ProcessStatistics() {
   // initialize overall threads stats
   RunThreadStats tstats;
   tstats.min = ULLONG_MAX;
   tstats.max = 0;
   tstats.sum = 0;
   tstats.avg = 0;
   tstats.med = 0;

   // calculate statistics
   size_t counter = 1;
   for (auto test : m_tests) {
      RunThreadStats stats;
      // process timings
      ProcessTimings(counter++, test, stats);
      // update overall stats
      tstats.sum += stats.sum;
      tstats.avg += stats.avg;
      tstats.med += stats.med;
      if (stats.min < tstats.min) tstats.min = stats.min;
      if (stats.max > tstats.max) tstats.max = stats.max;
   }
   // print min/min, max/max, avg/avg, avg/med for all threads
   if (counter > 0) {
      tstats.avg = tstats.avg / counter;
      tstats.med = tstats.med / counter;

      std::cout << "  ["
         << std::setw(2) << std::right << "**" << "] min/max/avg/med = "
         << std::setw(10) << std::right << FormatDuration(tstats.min) << " / "
         << std::setw(10) << std::right << FormatDuration(tstats.max) << " / "
         << std::setw(10) << std::right << FormatDuration(tstats.avg) << " / "
         << std::setw(10) << std::right << FormatDuration(tstats.med) << " / -"
         << std::endl;
   }
   // final statistics
   std::cout << std::endl << "Test \"" << m_name << "\" summ run time " << FormatDuration(tstats.sum) << std::endl;
   std::cout << "======================================================================================" << std::endl;

}
//+------------------------------------------------------------------+
//| Format duration as string                                        |
//+------------------------------------------------------------------+
std::string Test::FormatDuration(int64_t duration_ns) {
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
//| Calculate median                                                 |
//+------------------------------------------------------------------+
inline uint64_t TimingsMedian(const RunTestCfg::Timings& timings) {
   if (timings.empty()) return 0;

   std::vector<uint64_t> durations;
   durations.reserve(timings.size());
   for (const auto& t : timings)
      durations.push_back(t.duration);

   size_t n = durations.size();
   auto mid = durations.begin() + n / 2;

   std::nth_element(durations.begin(), mid, durations.end());

   if (n % 2 != 0) {
      return *mid;
   }
   else {
      // for an even-sized array, we need the second central element
      auto mid2 = *std::max_element(durations.begin(), mid);
      return (*mid + mid2) / 2;
   }
}
//+------------------------------------------------------------------+
//| Process timings from single thread                               |
//+------------------------------------------------------------------+
void Test::ProcessTimings(size_t id, RunTestCfg* test, RunThreadStats& stats) {
   // initialize stats
   stats.min = ULLONG_MAX;
   stats.max = 0;
   stats.avg = 0;
   stats.sum = 0;
   stats.med = 0;
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
      stats.med = TimingsMedian(test->timings);

      std::cout << "  ["
                << std::setw(2)  << std::right << id << "] min/max/avg/med = "
                << std::setw(10) << std::right << FormatDuration(stats.min) << " / "
                << std::setw(10) << std::right << FormatDuration(stats.max) << " / "
                << std::setw(10) << std::right << FormatDuration(stats.avg) << " / "
                << std::setw(10) << std::right << FormatDuration(stats.med) << " / "
                << (test->context_init.empty() ?"":("("+ test->context_init +") "))
                << (test->initializer.empty()?"-": test->initializer)
                << std::endl;
   }
   else {
      std::cout << "  ["
                << std::setw(2)  << std::right << id << "] min/max/avg/med = "
                << std::setw(52) << std::right << "/ "
                << (test->context_init.empty() ? "" : ("(" + test->context_init + ") "))
                << (test->initializer.empty() ? "-" : test->initializer)
                << std::endl;
   }
}
//+------------------------------------------------------------------+
