//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#pragma once
#include <windows.h>
#include <string>

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
//| DLL functions definitions                                        |
//+------------------------------------------------------------------+
typedef ITest* (*BtCreate_t) (const char* initializer);
typedef void   (*BtDestroy_t)(ITest* test);
typedef int    (*BtVersion_t)();

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class TestFactory {
private:
   HMODULE           m_lib;
   std::string       m_initializer;
   BtCreate_t        m_fnBtCreate;
   BtDestroy_t       m_fnBtDestroy;

public:
                     TestFactory();
                    ~TestFactory();

   bool              Load(LPCSTR path, LPCSTR initializer);

   ITest*            Create(LPCSTR initializer);
   void              Destroy(ITest* test);
};
// globals
extern char ExtProgramPath[MAX_PATH];
//+------------------------------------------------------------------+
