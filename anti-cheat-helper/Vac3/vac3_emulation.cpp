#include "vac3_emulation.h"
#include "../utils/module_utils.h"

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include "../utils/reporting.h"

#undef min

namespace vac_emulation {

	// Helper: get loaded modules as vector of module_range_t
	static std::vector<module_range_t> get_loaded_modules(DWORD pid) {
		std::vector<module_range_t> modules;

		std::vector<uintptr_t> starts;
		std::vector<uintptr_t> ends;
		std::vector<std::wstring> names;
		utils::get_module_ranges(pid, starts, ends, names);

		size_t count = std::min({ starts.size(), ends.size(), names.size() });
		for (size_t i = 0; i < count; ++i) {
			modules.push_back(module_range_t{ starts[i], ends[i], names[i] });
		}

		return modules;
	}

	// Helper: is address in any module range
	static bool is_address_in_modules(uintptr_t addr, const std::vector<module_range_t>& modules) {
		for (const auto& mod : modules) {
			if (addr >= mod.base && addr < mod.end)
				return true;
		}
		return false;
	}

	// Helper: is address in trusted module ranges (uses utils::is_trusted_module)
	static bool is_address_in_trusted_module(uintptr_t addr, const std::vector<module_range_t>& modules) {
		for (const auto& mod : modules) {
			if (addr >= mod.base && addr < mod.end) {
				if (utils::is_trusted_module(mod.name))
					return true;
			}
		}
		return false;
	}

	// Helper: check if region base is inside any loaded module
	static bool region_in_module_list(HANDLE hProc, void* addr, const std::vector<module_range_t>& modules) {
		return is_address_in_modules(reinterpret_cast<uintptr_t>(addr), modules);
	}

	// Helper: partial PE header check for suspicious memory regions
	static bool is_pe_header(const uint8_t* data, size_t size) {
		if (size < sizeof(IMAGE_DOS_HEADER))
			return false;
		if (data[0] != 'M' || data[1] != 'Z')
			return false;

		auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(data);
		if (dos->e_lfanew <= 0 || dos->e_lfanew > 0x1000 || dos->e_lfanew + sizeof(IMAGE_NT_HEADERS) > size)
			return false;

		auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(data + dos->e_lfanew);
		return nt->Signature == IMAGE_NT_SIGNATURE;
	}

	//caches
	static std::unordered_set<uintptr_t> g_reported_regions;
	static std::unordered_set<uintptr_t> g_reported_thread_starts;

	// Heuristic to skip known system/private executable memory regions, have do this for now 
	static const std::set<uintptr_t> known_safe_exec_regions = {
		0x7ffbfbe70000, // .NET Core or CLR JIT region
		0x7ffc43840000, // App-specific loader 
		0x7ffc482e0000  // Another trusted loader blob
	};

	bool is_known_safe_exec_region(uintptr_t addr) {
		return known_safe_exec_regions.find(addr) != known_safe_exec_regions.end();
	}

	// Scan suspicious executable regions (manual maps, RWX, etc) with whitelist caches
    inline void scan_suspicious_exec_regions(HANDLE hProc, const std::vector<module_range_t>& modules) {
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

                if (is_known_safe_exec_region(baseAddr)) {
                    addr += mbi.RegionSize;
                    continue;
                }

                if (g_reported_regions.find(baseAddr) == g_reported_regions.end()) {
                    if (!region_in_module_list(hProc, mbi.BaseAddress, modules)) {
                        if (!is_address_in_trusted_module(baseAddr, modules)) {
                            std::vector<uint8_t> buffer(mbi.RegionSize);
                            SIZE_T bytesRead;
                            if (ReadProcessMemory(hProc, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                                bool has_pe = is_pe_header(buffer.data(), bytesRead);

                                bool has_code_pattern = false;
                                for (size_t i = 0; i + 5 < bytesRead; i++) {
                                    if ((buffer[i] == 0x55 && buffer[i + 1] == 0x8B) ||
                                        ((buffer[i] == 0x40 || buffer[i] == 0x48) && buffer[i + 1] == 0x89)) {
                                        has_code_pattern = true;
                                        break;
                                    }
                                }

                                if (has_pe || has_code_pattern) {
                                    // Build log line prefix
                                    utils::add_log("[manual] Suspicious exec region @ 0x%p Size: %zu KB %s%s%s",
                                        (void*)baseAddr,
                                        mbi.RegionSize / 1024,
                                        has_pe ? "[Partial PE Header] " : "",
                                        has_code_pattern && !has_pe ? "[Code pattern] " : "",
                                        (mbi.Protect & PAGE_EXECUTE_READWRITE) ? "[RWX] " : ((mbi.Protect & PAGE_EXECUTE_READ) ? "[RX] " : ""));

                                    // Build hex dump snippet
                                    size_t snippet_len = std::min<size_t>(32, bytesRead);
                                    std::string hex_snippet = "  Hex dump snippet: ";
                                    char hex_byte[4];
                                    for (size_t j = 0; j < snippet_len; j++) {
                                        snprintf(hex_byte, sizeof(hex_byte), "%02X ", buffer[j]);
                                        hex_snippet += hex_byte;
                                    }

                                    utils::add_log("%s", hex_snippet.c_str());

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

    void scan_suspicious_threads(DWORD pid, HANDLE hProc, const std::vector<module_range_t>& modules) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snap == INVALID_HANDLE_VALUE)
            return;

        THREADENTRY32 te{};
        te.dwSize = sizeof(te);

        if (Thread32First(snap, &te)) {
            do {
                if (te.th32OwnerProcessID != pid)
                    continue;

                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
                if (hThread) {
                    void* start = nullptr;

                    using NtQueryInformationThread_t = NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);
                    static NtQueryInformationThread_t NtQueryInformationThread = nullptr;

                    if (!NtQueryInformationThread)
                        NtQueryInformationThread = reinterpret_cast<NtQueryInformationThread_t>(
                            GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread"));

                    if (NtQueryInformationThread) {
                        NtQueryInformationThread(hThread, 9 /*ThreadQuerySetWin32StartAddress*/, &start, sizeof(start), nullptr);

                        uintptr_t start_addr = reinterpret_cast<uintptr_t>(start);

                        if (!is_address_in_modules(start_addr, modules) && !is_address_in_trusted_module(start_addr, modules)) {
                            if (g_reported_thread_starts.find(start_addr) == g_reported_thread_starts.end()) {
                                g_reported_thread_starts.insert(start_addr);

                                MEMORY_BASIC_INFORMATION mbi{};
                                if (VirtualQueryEx(hProc, start, &mbi, sizeof(mbi))) {
                                    utils::add_log("[manual] Suspicious thread (TID %lu) entry at 0x%p in region 0x%p",
                                        te.th32ThreadID,
                                        (void*)start_addr,
                                        mbi.BaseAddress);
                                }
                                else {
                                    utils::add_log("[manual] Suspicious thread (TID %lu) entry at 0x%p (region query failed)",
                                        te.th32ThreadID,
                                        (void*)start_addr);
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

	// Run the VAC3-like scanner on target process
	void run_vac_like_scanner(DWORD target_pid) {
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, target_pid);
		if (!hProc) {
			return;
		}

		auto modules = get_loaded_modules(target_pid);

		scan_suspicious_exec_regions(hProc, modules);
		scan_suspicious_threads(target_pid, hProc, modules);

		CloseHandle(hProc);
	}

} // namespace vac_emulation
