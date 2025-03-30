//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include <windows.h>
#include <string>

#define BENCH_API_VERSION 3
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
typedef ITest* (*BtCreate_t) (const char* initializer);

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class TestFactory {
private:
   HMODULE           m_lib;
   std::string       m_initializer;
   BtCreate_t        m_fnBtCreate;

public:
                     TestFactory();
                    ~TestFactory();

   bool              Load(LPCSTR path, LPCSTR initializer);

   ITest*            Create(LPCSTR initializer);
};
// globals
extern char ExtProgramPath[MAX_PATH];
//+------------------------------------------------------------------+
