//+------------------------------------------------------------------+
//|              Bench – a benchmark tool that runs tests as plugins |
//|                               Copyright (c) 2025, Arthur Valitov |
//|                                    https://github.com/arthur-cpp |
//+------------------------------------------------------------------+
#include "TestFactory.h"

// global variable
char ExtProgramPath[MAX_PATH] = "";
//+------------------------------------------------------------------+
//| Initialization                                                   |
//+------------------------------------------------------------------+
TestFactory::TestFactory() :m_lib(NULL),
                            m_fnBtCreate(NULL), m_fnBtDestroy(NULL) {
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

   // load DLL
   if ((m_lib = LoadLibrary(path)) != NULL) {
      // load functions
      BtVersion_t BtVersion = reinterpret_cast<BtVersion_t>(GetProcAddress(m_lib, "BtVersion"));
      m_fnBtCreate  = reinterpret_cast<BtCreate_t>(GetProcAddress(m_lib, "BtCreate"));
      m_fnBtDestroy = reinterpret_cast<BtDestroy_t>(GetProcAddress(m_lib, "BtDestroy"));

      // check functions pointers
      if (BtVersion && m_fnBtCreate && m_fnBtDestroy) {
         // check version
         if (BtVersion() == BENCH_API_VERSION) {
            // store initializer
            if (initializer)
               m_initializer = initializer;
            // everything is ok
            return true;
         }
      }
   }

   // something went wrong
   return false;
}
//+------------------------------------------------------------------+
//| Create test instance                                             |
//+------------------------------------------------------------------+
ITest* TestFactory::Create(LPCSTR initializer) {
   std::string init;
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
   return m_fnBtCreate ? m_fnBtCreate(init.empty() ? NULL : init.c_str()) : NULL;
}
//+------------------------------------------------------------------+
//| Destroy test instance                                            |
//+------------------------------------------------------------------+
void TestFactory::Destroy(ITest* test) {
   if (test && m_fnBtDestroy)
      m_fnBtDestroy(test);
}
//+------------------------------------------------------------------+
