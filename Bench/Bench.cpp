//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "Benchmark.h"
#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

//+------------------------------------------------------------------+
//| Main entry point                                                 |
//+------------------------------------------------------------------+
int main() {
   Benchmark bench;

   // check for leaks in debug mode
#if _DEBUG
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

   // prepare program path
   char* cp;
   GetModuleFileName(NULL, ExtProgramPath, _countof(ExtProgramPath));
   if ((cp = strrchr(ExtProgramPath, '\\')) != NULL) *cp = 0;
   
   // load config
   if (bench.LoadConfig()) {
      bench.Run();
   }

   // finish
   return 0;
}
//+------------------------------------------------------------------+
