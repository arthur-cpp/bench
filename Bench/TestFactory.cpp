//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "TestFactory.h"
#include <iostream>

// global variable
char ExtProgramPath[MAX_PATH] = "";
//+------------------------------------------------------------------+
//| Initialization                                                   |
//+------------------------------------------------------------------+
TestFactory::TestFactory() :m_lib(NULL), m_fnBtCreateTest(NULL),
                            m_fnBtCreateContext(NULL), m_fnDestroyContext(NULL) {
}
//+------------------------------------------------------------------+
//| Free resources                                                   |
//+------------------------------------------------------------------+
TestFactory::~TestFactory() {
   if (m_lib) FreeLibrary(m_lib);
}
//+------------------------------------------------------------------+
//| Load library and check it                                        |
//+------------------------------------------------------------------+
bool TestFactory::Load(LPCSTR path, LPCSTR initializer) {
   char filename[MAX_PATH];
   // checks
   if (!path || !initializer) return false;
   // prepare path
   _snprintf_s(filename, _countof(filename), _TRUNCATE, "%s\\tests\\%s", ExtProgramPath, path);

   // check if file exists
   if (GetFileAttributes(filename) == INVALID_FILE_ATTRIBUTES) {
      std::cerr << "Error: DLL not found at path: " << filename << std::endl;
      return false;
   }

   // load DLL
   if ((m_lib = LoadLibrary(filename)) != NULL) {
      // load functions
      BtVersion_t BtVersion= reinterpret_cast<BtVersion_t>(GetProcAddress(m_lib, "BtVersion"));
      m_fnBtCreateTest     = reinterpret_cast<BtCreateTest_t>(GetProcAddress(m_lib, "BtCreateTest"));
      m_fnBtCreateContext  = reinterpret_cast<BtCreateContext_t>(GetProcAddress(m_lib, "BtCreateContext"));
      m_fnDestroyContext   = reinterpret_cast<BtDestroyContext_t>(GetProcAddress(m_lib, "BtDestroyContext"));

      // check functions pointers
      if (BtVersion && m_fnBtCreateTest) {
         // check version
         int version = BtVersion();
         if (version == BENCH_API_VERSION) {
            // store initializer
            if (initializer)
               m_initializer = initializer;
            // everything is ok
            return true;
         }
         else
            std::cerr << "Error: DLL " << path << " version is " << version << ", but " << BENCH_API_VERSION << " is required" << std::endl;
      }
      else
         std::cerr << "Error: DLL " << path << " is missing required functions" << std::endl;
   }
   else
      std::cerr << "Error: Failed to load DLL " << path << " (error " << GetLastError() << ")" << std::endl;

   // something went wrong
   return false;
}
//+------------------------------------------------------------------+
//| Create test instance                                             |
//+------------------------------------------------------------------+
ITest* TestFactory::CreateTest(LPCSTR initializer, UINT64 context) {
   std::string init;
   // check
   if (!m_fnBtCreateTest) return NULL;
   // prepare initalization string
   if (m_initializer.empty()) init = initializer;
   else {
      init = m_initializer;
      if (initializer && *initializer != 0) {
         init.append(",");
         init.append(initializer);
      }
   }
   // create test instance
   return m_fnBtCreateTest(init.empty() ? NULL : init.c_str(), context);
}
//+------------------------------------------------------------------+
//| Create context                                                   |
//+------------------------------------------------------------------+
UINT64 TestFactory::CreateContext(LPCSTR initializer) {
   if (m_fnBtCreateContext) return m_fnBtCreateContext(initializer);
   return NULL;
}
//+------------------------------------------------------------------+
//| Destroy context                                                  |
//+------------------------------------------------------------------+
void TestFactory::DestroyContext(UINT64 context) {
   if (m_fnDestroyContext) m_fnDestroyContext(context);
}
//+------------------------------------------------------------------+
