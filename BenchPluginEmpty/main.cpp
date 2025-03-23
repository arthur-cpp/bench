//+------------------------------------------------------------------+
//|                                                     Bench Plugin |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "pch.h"

#define BENCH_API __declspec(dllexport)
#define BENCH_API_VERSION 2
//+------------------------------------------------------------------+
//| Interface to the test                                            |
//+------------------------------------------------------------------+
class ITest {
public:
   virtual int       RunBefore() = 0;
   virtual int       Run()       = 0;
   virtual int       RunAfter()  = 0;
};
//+------------------------------------------------------------------+
//| Example with empty implementation                                |
//+------------------------------------------------------------------+
class Test : public ITest {
   virtual int Run() {
      // MUST return TRUE to continue the test
      // return FALSE for something fatal to stop the test
      return TRUE;
   }

   virtual int RunBefore() { return TRUE; }
   virtual int RunAfter()  { return TRUE; }
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
   return TRUE;
}
//+------------------------------------------------------------------+
//| Create test object                                               |
//+------------------------------------------------------------------+
BENCH_API ITest* BtCreate(const char* initializer) {
   // instantiate test object
   Test* test = new Test();
   // return object
   return test;
}
//+------------------------------------------------------------------+
//| Destroy test object                                              |
//+------------------------------------------------------------------+
BENCH_API void BtDestroy(ITest* test) {
   if (test) {
      Test* ptr = dynamic_cast<Test*>(test);
      delete ptr;
   }
}
//+------------------------------------------------------------------+
//| Main entry point                                                 |
//+------------------------------------------------------------------+
BENCH_API int BtVersion() { return BENCH_API_VERSION; }
//+------------------------------------------------------------------+
