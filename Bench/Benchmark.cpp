//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include <iostream>
// yaml-cpp
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
// own headers
#include "Benchmark.h"

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Benchmark::Benchmark()  {}
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
      Test test;
      // initialize test
      if (test.Initialize(cfg)) {
         // run test
         test.Run();
         // calculate, show and store statistics
         test.ProcessStatistics();
      }
   }
}
//+------------------------------------------------------------------+
