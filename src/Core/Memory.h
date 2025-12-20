#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

// Disable zero-length array warnings
#pragma warning(push)
#pragma warning(disable : 4200)
#include "../Dependencies/MemProcFS/vmmdll.h"
#pragma warning(pop)

namespace ARC
{
    class Memory
    {
    public:
        static bool Initialize();
        static void Shutdown();

        static bool FindProcess(const std::string& processName);
        static uint64_t GetModuleBaseAddress(const std::string& moduleName = "");

        
        template <typename T>
        static T Read(uint64_t address)
        {
            T buffer{};
            if (!m_hVMM || !m_dwPID || !address) return buffer;

            DWORD bytesRead = 0;
            
            VMMDLL_MemReadEx(m_hVMM, m_dwPID, address, (PBYTE)&buffer, sizeof(T), &bytesRead, VMMDLL_FLAG_NOCACHE);
            return buffer;
        }

        static bool ReadBuffer(uint64_t address, void* buffer, size_t size);

        
        static inline VMM_HANDLE m_hVMM = nullptr;
        static inline DWORD m_dwPID = 0;
        static inline uint64_t m_BaseAddress = 0;
    };
}