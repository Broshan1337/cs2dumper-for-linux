#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace vac_emulation {

	// Represents a loaded module's memory range and name
	struct module_range_t {
		uintptr_t base;
		uintptr_t end;
		std::wstring name;
	};

	// Main entry point: runs all VAC3-like detection scans for the given process ID
	void run_vac_like_scanner(DWORD target_pid);

}