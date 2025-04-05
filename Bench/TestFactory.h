//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include <windows.h>
#include <string>

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
//| DLL functions definitions                                        |
//+------------------------------------------------------------------+
typedef int    (*BtVersion_t)();
typedef ITest* (*BtCreateTest_t) (const char* initializer, UINT64 context);
typedef UINT64 (*BtCreateContext_t)(const char* initializer);
typedef void   (*BtDestroyContext_t)(UINT64);
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class TestFactory {
private:
   HMODULE              m_lib;
   std::string          m_initializer;
   BtCreateTest_t       m_fnBtCreateTest;
   BtCreateContext_t    m_fnBtCreateContext;
   BtDestroyContext_t   m_fnDestroyContext;

public:
                        TestFactory();
                       ~TestFactory();

   bool                 Load(LPCSTR path, LPCSTR initializer);

   ITest*               CreateTest(LPCSTR initializer, UINT64 context);
   UINT64               CreateContext(LPCSTR initializer);
   void                 DestroyContext(UINT64 context);
};
// globals
extern char ExtProgramPath[MAX_PATH];
//+------------------------------------------------------------------+
