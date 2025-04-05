//+------------------------------------------------------------------+
//|                                                     Bench Plugin |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "pch.h"

#define BENCH_API __declspec(dllexport)
#define BENCH_API_VERSION 4
//+------------------------------------------------------------------+
//| Interface to the test                                            |
//+------------------------------------------------------------------+
class ITest {
public:
   virtual void      Release()   =0;   // release object

   virtual int       RunBefore() = 0;  // before test
   virtual int       Run()       = 0;  // measured test function
   virtual int       RunAfter()  = 0;  // after test
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

   virtual void Release() {
      delete this;
   }

   virtual int RunBefore() { return TRUE; }
   virtual int RunAfter()  { return TRUE; }
};
//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
   return TRUE;
}
//+------------------------------------------------------------------+
//| Bench API version                                                |
//+------------------------------------------------------------------+
BENCH_API int BtVersion() { return BENCH_API_VERSION; }
//+------------------------------------------------------------------+
//| Create test object                                               |
//+------------------------------------------------------------------+
BENCH_API ITest* BtCreateTest(const char* initializer, UINT64 context) {
   // instantiate test object
   Test* test = new Test();
   // return an object
   return test;
}
//+------------------------------------------------------------------+
//| Create context                                                   |
//+------------------------------------------------------------------+
BENCH_API UINT64 BtCreateContext(const char* initializer) {
   // you could create an object and return its pointer
   // this sample plugin returns a dummy number
   return 0xDEADBEAF;
}
//+------------------------------------------------------------------+
//| Destroy context                                                  |
//+------------------------------------------------------------------+
BENCH_API void BtDestroyContext(UINT64) {
   // destroy the context object
   // this sample plugin does nothing with the context
}
//+------------------------------------------------------------------+
