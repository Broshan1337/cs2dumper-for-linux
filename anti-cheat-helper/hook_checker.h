// hook_checker.h
#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace hook_checker {

    struct relocation_block {
        uint32_t page_rva = 0;
        uint32_t block_size = 0;
        std::vector<uint16_t> offsets;
    };

    struct section_info {
        uintptr_t base_address = 0;         // RVA of .text section in module
        size_t size = 0;                    // SizeOfRawData
        size_t virtual_size = 0;            // VirtualSize
        std::vector<uint8_t> original_bytes; // Snapshot bytes
        std::vector<relocation_block> relocations;
    };

    struct hook_detection {
        size_t offset = 0;
        uintptr_t target = 0;
        std::wstring type;
    };

    // Function declarations here (optional, but must be consistent)
    bool address_in_any_module(uintptr_t addr, const std::vector<uintptr_t>& starts, const std::vector<uintptr_t>& ends);
    DWORD get_process_id_by_name(const wchar_t* process_name);
    bool get_process_exe_path(DWORD pid, std::wstring& out_path);
    bool load_text_section_snapshot(const std::wstring& exe_path, section_info& out_section, uintptr_t module_base);
    void apply_relocations_to_snapshot(section_info& section, uintptr_t module_base);
    std::vector<hook_detection> check_for_hooks(
        DWORD pid,
        const section_info& section,
        uintptr_t module_base,
        const std::vector<uintptr_t>& mod_starts,
        const std::vector<uintptr_t>& mod_ends);
    bool is_game_module(const std::wstring& mod_name);
    void add_whitelist_offset(size_t offset, const std::wstring& reason);
    bool is_whitelisted_offset(size_t offset);

} // namespace hook_checker
