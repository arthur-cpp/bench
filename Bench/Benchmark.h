//+------------------------------------------------------------------+
//|              Bench - a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include "Test.h"

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
};
//+------------------------------------------------------------------+
