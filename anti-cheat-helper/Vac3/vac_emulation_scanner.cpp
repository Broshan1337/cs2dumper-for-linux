#include "vac_emulation_scanner.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <string>
#include <sstream>
#include <algorithm>

namespace vac_emulation {

    // --- Utility: lowercase wstring
    inline std::wstring to_lower(const std::wstring& s) {
        std::wstring out = s;
        std::transform(out.begin(), out.end(), out.begin(), towlower);
        return out;
    }

    // --- Trusted modules whitelist
    inline bool is_trusted_module(const std::wstring& modName) {
        static const std::set<std::wstring> trusted = {
            L"ntdll.dll",
            L"kernel32.dll",
            L"kernelbase.dll",
            L"user32.dll",
            L"gdi32.dll",
            L"win32u.dll",
            L"advapi32.dll",
            L"msvcrt.dll",
            L"mscoree.dll",
            L"clrjit.dll",
            L"clr.dll",
            L"shell32.dll",
            L"shcore.dll",
            L"combase.dll",
            L"rpcrt4.dll",
            L"ole32.dll",
            L"oleaut32.dll",
            L"crypt32.dll",
            L"secur32.dll",
            L"ws2_32.dll",
            L"wininet.dll",
            L"dbghelp.dll",
            L"psapi.dll",
            L"ntmarta.dll",
            L"uxtheme.dll",
            L"d3d11.dll",
            L"d3d9.dll",
            L"dwmapi.dll",
            L"dxgi.dll",
            L"dxcore.dll",
            L"msimg32.dll",
            L"kernel.appcore.dll",
            L"powrprof.dll",
            L"profapi.dll",
            L"sechost.dll",
            L"shlwapi.dll",
            L"version.dll",
            L"winmm.dll",
            L"wow64.dll",
            L"wow64win.dll",
            L"wow64cpu.dll",
            L"bcryptprimitives.dll",
            L"bcrypt.dll"
            // Add more if needed
        };

        std::wstring lowerName = to_lower(modName);
        for (const auto& tmod : trusted) {
            if (lowerName.find(tmod) != std::wstring::npos)
                return true;
        }
        return false;
    }

    // --- Struct representing a loaded module range and name
    struct ModuleRange {
        uintptr_t base;
        uintptr_t end;
        std::wstring name;
    };

    // --- Get all modules loaded in the target process
    inline std::vector<ModuleRange> get_loaded_modules(HANDLE hProc) {
        std::vector<ModuleRange> modules;
        HMODULE hMods[1024];
        DWORD cbNeeded;

        if (EnumProcessModules(hProc, hMods, sizeof(hMods), &cbNeeded)) {
            size_t count = cbNeeded / sizeof(HMODULE);
            wchar_t modName[MAX_PATH];
            for (size_t i = 0; i < count; i++) {
                if (GetModuleFileNameExW(hProc, hMods[i], modName, MAX_PATH)) {
                    MODULEINFO modInfo;
                    if (GetModuleInformation(hProc, hMods[i], &modInfo, sizeof(modInfo))) {
                        modules.push_back(ModuleRange{
                            reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll),
                            reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll) + modInfo.SizeOfImage,
                            std::wstring(modName)
                            });
                    }
                }
            }
        }
        return modules;
    }

    // --- Check if address is inside any module range
    inline bool is_address_in_modules(uintptr_t addr, const std::vector<ModuleRange>& modules) {
        for (const auto& mod : modules) {
            if (addr >= mod.base && addr < mod.end)
                return true;
        }
        return false;
    }

    // --- Check if address inside trusted module ranges
    inline bool is_address_in_trusted_module(uintptr_t addr, const std::vector<ModuleRange>& modules) {
        for (const auto& mod : modules) {
            if (addr >= mod.base && addr < mod.end) {
                if (is_trusted_module(mod.name))
                    return true;
            }
        }
        return false;
    }

    // --- Check if a region base is inside any loaded module (general)
    inline bool region_in_module_list(HANDLE hProc, void* addr, const std::vector<ModuleRange>& modules) {
        return is_address_in_modules(reinterpret_cast<uintptr_t>(addr), modules);
    }

    // --- Check if data looks like PE header (partial)
    inline bool is_pe_header(const uint8_t* data, size_t size) {
        if (size < sizeof(IMAGE_DOS_HEADER)) return false;
        if (data[0] != 'M' || data[1] != 'Z') return false;

        auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(data);
        if (dos->e_lfanew <= 0 || dos->e_lfanew > 0x1000 || dos->e_lfanew + sizeof(IMAGE_NT_HEADERS) > size)
            return false;

        auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(data + dos->e_lfanew);
        return nt->Signature == IMAGE_NT_SIGNATURE;
    }

    // --- Deduplication cache for suspicious regions
    static std::unordered_set<uintptr_t> g_reported_regions;
    static std::unordered_set<uintptr_t> g_reported_thread_starts;

    // --- Scan suspicious executable regions (manual maps, RWX, etc) with whitelist and dedupe
    inline void scan_suspicious_exec_regions(HANDLE hProc, const std::vector<ModuleRange>& modules) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        uint8_t* addr = (uint8_t*)sysInfo.lpMinimumApplicationAddress;
        uint8_t* max = (uint8_t*)sysInfo.lpMaximumApplicationAddress;

        MEMORY_BASIC_INFORMATION mbi;

        while (addr < max) {
            if (!VirtualQueryEx(hProc, addr, &mbi, sizeof(mbi))) break;

            bool is_exec = (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;

            if (mbi.State == MEM_COMMIT && is_exec && mbi.Type == MEM_PRIVATE) {
                uintptr_t baseAddr = reinterpret_cast<uintptr_t>(mbi.BaseAddress);

                // Deduplication: only report once per base address
                if (g_reported_regions.find(baseAddr) == g_reported_regions.end()) {
                    // Skip if region belongs to loaded modules
                    if (!region_in_module_list(hProc, mbi.BaseAddress, modules)) {
                        // Skip if inside trusted module ranges
                        if (!is_address_in_trusted_module(baseAddr, modules)) {
                            std::vector<uint8_t> buffer(mbi.RegionSize);
                            SIZE_T bytesRead;
                            if (ReadProcessMemory(hProc, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                                bool has_pe = is_pe_header(buffer.data(), bytesRead);

                                bool has_code_pattern = false;
                                for (size_t i = 0; i + 5 < bytesRead; i++) {
                                    if ((buffer[i] == 0x55 && buffer[i + 1] == 0x8B) || // push ebp; mov ebp, esp
                                        ((buffer[i] == 0x40 || buffer[i] == 0x48) && buffer[i + 1] == 0x89)) { // mov with REX prefix
                                        has_code_pattern = true;
                                        break;
                                    }
                                }

                                if (has_pe || has_code_pattern) {
                                    std::wstringstream ws;
                                    ws << L"[VAC3-LIKE] Suspicious exec region @ 0x" << std::hex << baseAddr
                                        << L" Size: " << std::dec << mbi.RegionSize / 1024 << L" KB ";

                                    if (has_pe) ws << L"[Partial PE Header] ";
                                    else if (has_code_pattern) ws << L"[Code pattern] ";

                                    if (mbi.Protect & PAGE_EXECUTE_READWRITE) ws << L"[RWX] ";
                                    else if (mbi.Protect & PAGE_EXECUTE_READ) ws << L"[RX] ";

                                    std::wcout << ws.str() << L"\n";

                                    size_t snippet_len = std::min<size_t>(32, bytesRead);
                                    std::wcout << L"  Hex dump snippet: ";
                                    for (size_t j = 0; j < snippet_len; j++) {
                                        std::wcout << std::hex << (int)buffer[j] << L" ";
                                    }
                                    std::wcout << std::dec << L"\n";

                                    g_reported_regions.insert(baseAddr);
                                }
                            }
                        }
                    }
                }
            }
            addr += mbi.RegionSize;
        }
    }

    // --- Scan threads for suspicious thread start addresses outside modules with deduplication and whitelist
    inline void scan_suspicious_threads(DWORD pid, HANDLE hProc, const std::vector<ModuleRange>& modules) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snap == INVALID_HANDLE_VALUE) return;

        THREADENTRY32 te = { sizeof(te) };

        if (Thread32First(snap, &te)) {
            do {
                if (te.th32OwnerProcessID != pid) continue;

                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
                if (hThread) {
                    void* start = nullptr;

                    static auto NtQueryInformationThread = (NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(
                        GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread");

                    if (NtQueryInformationThread) {
                        NtQueryInformationThread(hThread, 9 /*ThreadQuerySetWin32StartAddress*/, &start, sizeof(start), nullptr);

                        uintptr_t start_addr = reinterpret_cast<uintptr_t>(start);

                        // Skip if start address inside any loaded module or trusted module
                        if (!is_address_in_modules(start_addr, modules) && !is_address_in_trusted_module(start_addr, modules)) {
                            if (g_reported_thread_starts.find(start_addr) == g_reported_thread_starts.end()) {
                                g_reported_thread_starts.insert(start_addr);

                                MEMORY_BASIC_INFORMATION mbi;
                                if (VirtualQueryEx(hProc, start, &mbi, sizeof(mbi))) {
                                    std::wcout << L"[VAC3-LIKE] Suspicious thread (TID " << te.th32ThreadID << L") entry at 0x" << std::hex
                                        << start_addr << L" in region 0x" << reinterpret_cast<uintptr_t>(mbi.BaseAddress) << std::dec << L"\n";
                                }
                                else {
                                    std::wcout << L"[VAC3-LIKE] Suspicious thread (TID " << te.th32ThreadID << L") entry at 0x" << std::hex
                                        << start_addr << L" (region query failed)" << std::dec << L"\n";
                                }
                            }
                        }
                    }
                    CloseHandle(hThread);
                }
            } while (Thread32Next(snap, &te));
        }
        CloseHandle(snap);
    }

    // --- Run the VAC3-like scanner on target process
    void run_vac_like_scanner(DWORD target_pid) {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, target_pid);
        if (!hProc) {
            std::wcerr << L"[ERROR] Failed to open target process\n";
            return;
        }

        auto modules = get_loaded_modules(hProc);

        scan_suspicious_exec_regions(hProc, modules);
        scan_suspicious_threads(target_pid, hProc, modules);

        CloseHandle(hProc);
    }

} // namespace vac_emulation
