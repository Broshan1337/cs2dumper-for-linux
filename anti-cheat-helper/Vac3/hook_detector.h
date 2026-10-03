#pragma once

#include <Windows.h>
#include <string>
#include <vector>

namespace vac3 {

	struct relocation_block_t {
		uint32_t page_rva = 0;
		uint32_t block_size = 0;
		std::vector<uint16_t> offsets;
	};

	struct section_info_t {
		uintptr_t base_address = 0;
		size_t size = 0;
		size_t virtual_size = 0;
		std::vector<uint8_t> original_bytes;
		std::vector<relocation_block_t> relocations;
	};

	struct hook_detection_t {
		size_t offset = 0;           // Offset inside the section
		uintptr_t target = 0;        // Target address the hook jumps to
		std::wstring type;           // Hook type string, e.g. "jmp out of module"
	};

	// Load the .text section snapshot from disk for a module path and image base
	bool load_text_section_snapshot(const std::wstring& exe_path, section_info_t& out_section, uintptr_t image_base);

	// Check for hooks given a process handle, section info, and loaded modules info
	std::vector<hook_detection_t> check_for_hooks(
		DWORD pid,
		const section_info_t& section,
		uintptr_t module_base,
		const std::vector<uintptr_t>& mod_starts,
		const std::vector<uintptr_t>& mod_ends);

	// Check if an address lies within any known module range
	bool address_in_any_module(uintptr_t addr, const std::vector<uintptr_t>& starts, const std::vector<uintptr_t>& ends);
} // namespace vac3