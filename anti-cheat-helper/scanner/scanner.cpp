// scanner.cpp
#include "scanner.h"
#include "../utils/module_utils.h"
#include "../Vac3/hook_detector.h"
#include "../Vac3/vac3_emulation.h"
#include "../utils/reporting.h"
#include <Psapi.h>
#include <cstdarg>
#include <map>
#include <array>

namespace scanner {
    std::atomic<bool> is_scanning{ false };
    std::atomic<bool> stop_scanning{ false };

    std::vector<std::wstring> GetLogLines() {
        std::lock_guard<std::mutex> lock(utils::log_mutex);
        return utils::log_lines;
    }

    DWORD WaitForProcess(const wchar_t* process_name) {
        DWORD pid = 0;
        while ((pid = utils::get_process_id_by_name(process_name)) == 0) {
            Sleep(1000);
        }
        return pid;
    }

    int do_scan(DWORD pid) {

        std::wstring exe_path;
        if (!utils::get_process_exe_path(pid, exe_path)) {
            utils::add_log("[ERROR] Failed to get executable path");
            return 1;
        }

        std::map<std::wstring, vac3::section_info_t> snapshots;
        std::map<std::wstring, uintptr_t> module_bases;

        HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!process_handle) {
            utils::add_log("[ERROR] Failed to open process handle (error: %lu)", GetLastError());
            return 1;
        }

        std::vector<HMODULE> modules;
        if (!utils::enum_process_modules(pid, modules)) {
            utils::add_log("[ERROR] Failed to enumerate modules");
            CloseHandle(process_handle);
            return 1;
        }

        for (const auto mod : modules) {
            wchar_t mod_name[MAX_PATH]{};
            if (!GetModuleBaseNameW(process_handle, mod, mod_name, MAX_PATH))
                continue;

            const std::wstring module_name{ mod_name };
            if (!utils::is_game_module(module_name))
                continue;

            wchar_t mod_path[MAX_PATH]{};
            if (!GetModuleFileNameExW(process_handle, mod, mod_path, MAX_PATH)) {
                //std::wcerr << L"[WARN] Failed to get module path for " << module_name << L'\n';
                continue;
            }

            const uintptr_t module_base = reinterpret_cast<uintptr_t>(mod);

            vac3::section_info_t sec_info{};
            if (!vac3::load_text_section_snapshot(mod_path, sec_info, module_base)) {
                //std::wcerr << L"[WARN] Failed to load snapshot for " << module_name << L'\n';
                continue;
            }

            module_bases[module_name] = module_base;
            snapshots[module_name] = std::move(sec_info);

            if (_wcsicmp(module_name.c_str(), L"tier0.dll") == 0) {
                constexpr std::array<uintptr_t, 8> whitelist_offsets{{
                    0x1B4116u, 0x1B4117u, 0x1B4118u, 0x1B4119u,
                    0x1B41B7u, 0x1B41B8u, 0x1B41B9u, 0x1B41BAu
                }};
                for (const auto& off : whitelist_offsets)
                    utils::add_whitelist_offset(off);
            }
        }

        bool found_hooks = false;

        std::vector<uintptr_t> mod_starts;
        std::vector<uintptr_t> mod_ends;
        std::vector<std::wstring> mod_names;

        utils::get_module_ranges(pid, mod_starts, mod_ends, mod_names);

        for (const auto& [module_name, section] : snapshots) {
            const auto base = module_bases.at(module_name);
            const auto hooks = vac3::check_for_hooks(pid, section, base, mod_starts, mod_ends);

            if (!hooks.empty()) {
                found_hooks = true;
                utils::add_log("[hook] [ALERT] Hooks detected in module: %ws", module_name.c_str());

                for (const auto& hook : hooks) {
                    utils::add_log("[hook] [%ws] at offset 0x%llX jumping to 0x%llX",
                        hook.type.c_str(), hook.offset, hook.target);
                }
            }
        }

        vac_emulation::run_vac_like_scanner(pid);

        if (found_hooks) {
            utils::add_log("Scan completed. Hooks were found.");
        }
        else {
            utils::add_log("Scan completed. No hooks detected.");
        }

        return found_hooks ? 1 : 0;
    }
}