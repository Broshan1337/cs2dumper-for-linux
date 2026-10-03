// scanner.h
#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <Windows.h>

namespace scanner {
    std::vector<std::wstring> GetLogLines();

    extern std::atomic<bool> is_scanning;
    extern std::atomic<bool> stop_scanning;

    DWORD WaitForProcess(const wchar_t* process_name);
    int do_scan(DWORD pid);
}
