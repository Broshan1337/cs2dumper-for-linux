#pragma once
#include <string>
#include <iostream>
#include <mutex>
#include <atomic>
#include <Windows.h>
#include <vector>

namespace utils {

	extern std::mutex log_mutex;
	extern std::vector<std::wstring> log_lines;

	void report_hook(const std::wstring& module, const std::wstring& type, uintptr_t offset, uintptr_t target);;

	void report_exec_region(uintptr_t address, size_t size, const std::string& perms, const std::string& hex_snippet);

	void add_log(const char* fmt, ...);

} // namespace utils
