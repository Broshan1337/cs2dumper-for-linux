#pragma once
#include <Windows.h>

namespace vac_emulation {

	// Call this with the target process ID to run all VAC-style detection scans.
	// It prints results to console.
	void run_vac_like_scanner(DWORD target_pid);

}