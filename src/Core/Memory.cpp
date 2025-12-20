#include "Memory.h"
#include <thread>
#include <chrono>
#include <iostream>

namespace ARC
{
    bool Memory::Initialize()
    {
        LPCSTR args[] = { "", "-device", "fpga" };
        m_hVMM = VMMDLL_Initialize(3, args);

        if (!m_hVMM)
        {
            std::cout << "[!] Failed to initialize VMMDLL. Check FPGA connection." << std::endl;
            return false;
        }

        std::cout << "[+] DMA Connected!" << std::endl;
        std::cout << "[*] Waiting for PioneerGame.exe..." << std::endl;

        while (!VMMDLL_PidGetFromName(m_hVMM, (LPSTR)"PioneerGame.exe", &m_dwPID))
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "[+] Process found! PID: " << m_dwPID << std::endl;

        m_BaseAddress = VMMDLL_ProcessGetModuleBaseU(m_hVMM, m_dwPID, (LPSTR)"PioneerGame.exe");
        std::cout << "[+] Base Address: 0x" << std::hex << m_BaseAddress << std::dec << std::endl;

        return true;
    }

    void Memory::Shutdown()
    {
        if (m_hVMM)
        {
            VMMDLL_Close(m_hVMM);
            m_hVMM = nullptr;
            m_dwPID = 0;
        }
    }

    bool Memory::FindProcess(const std::string& processName)
    {
        if (!m_hVMM) return false;
        return VMMDLL_PidGetFromName(m_hVMM, (LPSTR)processName.c_str(), &m_dwPID);
    }

    uint64_t Memory::GetModuleBaseAddress(const std::string& moduleName)
    {
        if (!m_hVMM || !m_dwPID) return 0;
        if (moduleName.empty()) return m_BaseAddress;
        return VMMDLL_ProcessGetModuleBaseU(m_hVMM, m_dwPID, (LPSTR)moduleName.c_str());
    }

    bool Memory::ReadBuffer(uint64_t address, void* buffer, size_t size)
    {
        if (!m_hVMM || !m_dwPID || !address) return false;
        DWORD bytesRead = 0;
        return VMMDLL_MemReadEx(m_hVMM, m_dwPID, address, (PBYTE)buffer, (DWORD)size, &bytesRead, VMMDLL_FLAG_NOCACHE);
    }
}