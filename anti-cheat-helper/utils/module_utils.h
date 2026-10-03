// utils/module_utils.h
#pragma once
#include <vector>
#include <string>
#include <set>
#include <Windows.h>

namespace utils {

	bool enum_process_modules(DWORD pid, std::vector<HMODULE>& out_modules);

	bool get_module_info(HMODULE hMod, uintptr_t& base, size_t& size, std::wstring& mod_name);

	void get_module_ranges(DWORD pid, std::vector<uintptr_t>& out_starts, std::vector<uintptr_t>& out_ends, std::vector<std::wstring>& out_names);

	// Convert wide string to lowercase
	std::wstring to_lower(const std::wstring& s);

	bool get_process_exe_path(DWORD pid, std::wstring& out_path);

	// Return PID for process name, or 0 if not found
	DWORD get_process_id_by_name(const wchar_t* process_name);

	// Returns true if the module name is in the trusted whitelist
	bool is_trusted_module(const std::wstring& mod_name);

	// Check if a module name is part of the game modules to scan
	bool is_game_module(const std::wstring& mod_name);

	// Whitelist offsets that are known safe
	void add_whitelist_offset(size_t offset);

	// Check if offset is whitelisted
	bool is_whitelisted_offset(size_t offset);

} // namespace utils