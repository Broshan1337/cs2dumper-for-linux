// anti-cheat helper.cpp : This file contains the 'main' function. Program execution begins and ends there.
// hook_detector.cpp
// Full working hook detector for inline hooks in CS2 module
// this scans the cs2 .text file similar to how valves anticheat does it for cs2

#include "decoder/decoder.h"
#include "hook_checker.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>

#undef min

namespace hook_checker {

	static std::map<size_t, std::wstring> whitelisted_offsets;

	DWORD get_process_id_by_name(const wchar_t* process_name) {
		auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snap == INVALID_HANDLE_VALUE) return 0;

		PROCESSENTRY32W pe{};
		pe.dwSize = sizeof(pe);
		DWORD pid = 0;
		if (Process32FirstW(snap, &pe)) {
			do {
				if (_wcsicmp(pe.szExeFile, process_name) == 0) {
					pid = pe.th32ProcessID;
					break;
				}
			} while (Process32NextW(snap, &pe));
		}
		CloseHandle(snap);
		return pid;
	}

	bool get_process_exe_path(DWORD pid, std::wstring& out_path) {
		auto h_process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (!h_process) return false;

		wchar_t buffer[MAX_PATH]{};
		DWORD size = MAX_PATH;
		auto success = QueryFullProcessImageNameW(h_process, 0, buffer, &size) == TRUE;
		if (success) out_path.assign(buffer, size);

		CloseHandle(h_process);
		return success;
	}

	static std::vector<relocation_block> parse_relocations(const uint8_t* base, size_t file_size) {
		std::vector<relocation_block> blocks;

		if (file_size < sizeof(IMAGE_DOS_HEADER))
			return blocks;

		auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return blocks;

		if (dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > file_size)
			return blocks;

		auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return blocks;

		auto& data_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
		if (data_dir.Size == 0)
			return blocks;

		if (data_dir.VirtualAddress + data_dir.Size > file_size)
			return blocks;

		const uint8_t* reloc_base = base + data_dir.VirtualAddress;
		const uint8_t* reloc_end = reloc_base + data_dir.Size;

		const uint8_t* reloc_ptr = reloc_base;

		while (reloc_ptr + sizeof(IMAGE_BASE_RELOCATION) <= reloc_end) {
			auto reloc = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(reloc_ptr);

			if (reloc->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION))
				break;

			if (reloc_ptr + reloc->SizeOfBlock > reloc_end)
				break;

			relocation_block block;
			block.page_rva = reloc->VirtualAddress;
			block.block_size = reloc->SizeOfBlock;

			size_t entry_count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			const WORD* entries = reinterpret_cast<const WORD*>(reloc + 1);

			for (size_t i = 0; i < entry_count; ++i) {
				WORD entry = entries[i];
				WORD type = entry >> 12;
				WORD offset = entry & 0xFFF;

				// Only apply 64-bit relocations (x64)
				if (type == IMAGE_REL_BASED_DIR64)
					block.offsets.push_back(offset);
			}

			blocks.push_back(std::move(block));
			reloc_ptr += reloc->SizeOfBlock;
		}

		return blocks;
	}

	std::vector<uint8_t> load_file(const std::wstring& path) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
			return {};

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
			return {};

		return buffer;
	}

	// Apply relocations to the snapshot bytes in place:
	void apply_relocations_to_snapshot(section_info& section, uintptr_t module_base) {
		for (const auto& block : section.relocations) {
			for (auto offset : block.offsets) {
				uint32_t reloc_rva = block.page_rva + offset;

				if (reloc_rva < section.base_address ||
					reloc_rva + sizeof(uint64_t) > section.base_address + section.original_bytes.size())
					continue; // Skip relocation outside of .text

				size_t reloc_offset = reloc_rva - section.base_address;

				uint64_t original_value = 0;
				memcpy(&original_value, &section.original_bytes[reloc_offset], sizeof(uint64_t));
				original_value += module_base;
				memcpy(&section.original_bytes[reloc_offset], &original_value, sizeof(uint64_t));
			}
		}
	}


	bool load_text_section_snapshot(const std::wstring& exe_path, section_info& out_section, uintptr_t image_base) {
		auto h_file = CreateFileW(exe_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (h_file == INVALID_HANDLE_VALUE) return false;

		auto h_mapping = CreateFileMappingW(h_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (!h_mapping) {
			CloseHandle(h_file);
			return false;
		}

		auto base = reinterpret_cast<uint8_t*>(MapViewOfFile(h_mapping, FILE_MAP_READ, 0, 0, 0));
		if (!base) {
			CloseHandle(h_mapping);
			CloseHandle(h_file);
			return false;
		}

		auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
		auto section = IMAGE_FIRST_SECTION(nt);

		bool found = false;
		for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; i++, section++) {
			if (strncmp(reinterpret_cast<const char*>(section->Name), ".text", 5) == 0) {
				out_section.base_address = section->VirtualAddress;
				out_section.size = section->SizeOfRawData;
				out_section.virtual_size = section->Misc.VirtualSize;

				auto src = base + section->PointerToRawData;
				out_section.original_bytes.assign(src, src + out_section.size);

				found = true;
				break;
			}
		}

		auto file_data = load_file(exe_path);
		if (file_data.empty()) {
			UnmapViewOfFile(base);
			CloseHandle(h_mapping);
			CloseHandle(h_file);
			return false;
		}

		//out_section.relocations = parse_relocations(file_data.data(), file_data.size());

		////apply_relocations_to_snapshot(out_section, image_base);

		UnmapViewOfFile(base);
		CloseHandle(h_mapping);
		CloseHandle(h_file);
		return found;
	}

	bool is_game_module(const std::wstring& mod_name) {
		static const std::wstring game_modules[] = {
			L"cs2.exe", L"engine2.dll", L"client.dll", L"rendersystemdx11.dll",
			L"materialsystem2.dll", L"inputsystem.dll", L"vphysics2.dll",
			L"soundsystem.dll", L"tier0.dll", L"vstdlib.dll", L"filesystem_stdio2.dll",
			L"steamnetworkingsockets.dll", L"matchmaking.dll", L"datacache.dll",
			L"studiorender.dll", L"shaderapidx11.dll"
		};
		for (const auto& gm : game_modules) {
			if (_wcsicmp(gm.c_str(), mod_name.c_str()) == 0)
				return true;
		}
		return false;
	}

	void add_whitelist_offset(size_t offset, const std::wstring& reason) {
		whitelisted_offsets[offset] = reason;
	}

	bool is_whitelisted_offset(size_t offset) {
		return whitelisted_offsets.find(offset) != whitelisted_offsets.end();
	}

	bool hook_checker::address_in_any_module(uintptr_t addr, const std::vector<uintptr_t>& starts, const std::vector<uintptr_t>& ends) {
		for (size_t i = 0; i < starts.size(); ++i) {
			if (addr >= starts[i] && addr < ends[i])
				return true;
		}
		return false;
	}

	static std::mutex detected_hooks_mutex;

	void scan_section_for_hooks_thread(
		HANDLE h_process,
		uintptr_t text_mem_addr,
		const section_info& section,
		uintptr_t module_base,
		const std::vector<uintptr_t>& mod_starts,
		const std::vector<uintptr_t>& mod_ends,
		size_t start,
		size_t end,
		std::vector<hook_detection>& detected_hooks,
		std::mutex& detected_hooks_mutex)
	{
		std::vector<hook_detection> local_detected;

		std::vector<uint8_t> buffer(end - start);
		SIZE_T bytes_read = 0;

		if (!ReadProcessMemory(h_process, reinterpret_cast<LPCVOID>(text_mem_addr + start), buffer.data(), buffer.size(), &bytes_read))
			return;

		const size_t compare_size = std::min<size_t>(bytes_read, section.original_bytes.size() - start);

		for (size_t i = 0; i + 14 < compare_size;) {
			if (buffer[i] != section.original_bytes[start + i] && !is_whitelisted_offset(start + i)) {
				decoder::Instruction inst;
				decoder::Decoder decoder_instance;
				bool decoded = decoder_instance.decode(buffer.data() + i, compare_size - i, inst);

				if (decoded && inst.length > 0) {
					uintptr_t instr_addr = text_mem_addr + start + i;
					uintptr_t hook_target = 0;
					bool is_hook = false;

					// 1. Enhanced indirect JMP thunk detection
					if (inst.mnemonic == "JMP") {
						if (!inst.operands.empty()) {
							auto& op = inst.operands[0];
							if (op.type == decoder::OperandType::IMM) {
								int64_t rel = static_cast<int64_t>(op.immediate);
								hook_target = instr_addr + inst.length + rel;
								if (hook_target < module_base || hook_target > module_base + section.virtual_size) {
									local_detected.push_back({ start + i, hook_target, L"JMP rel32 (likely hook)" });
									is_hook = true;
								}
							}
							else if (op.type == decoder::OperandType::MEM) {
								local_detected.push_back({ start + i, 0, L"Indirect JMP (hook thunk)" });
								is_hook = true;
							}
						}
					}
					else if (inst.mnemonic == "CALL") {
						if (!inst.operands.empty()) {
							auto& op = inst.operands[0];
							if (op.type == decoder::OperandType::IMM) {
								int64_t rel = static_cast<int64_t>(op.immediate);
								hook_target = instr_addr + inst.length + rel;
								if (hook_target < module_base || hook_target > module_base + section.virtual_size) {
									local_detected.push_back({ start + i, hook_target, L"CALL rel32 (likely hook)" });
									is_hook = true;
								}
							}
						}
					}

					// 2. Improved Detours detection: MOV RAX imm64 + JMP RAX sequence
					if (!is_hook && inst.mnemonic == "MOV" && inst.length >= 10) {
						if (inst.operands.size() == 2
							&& inst.operands[0].type == decoder::OperandType::REG
							&& inst.operands[0].reg == "rax"
							&& inst.operands[1].type == decoder::OperandType::IMM
							&& inst.operands[1].size == 8)
						{
							size_t next_offset = i + inst.length;
							size_t max_scan = std::min<size_t>(compare_size - next_offset, 20);
							size_t scanned_bytes = 0;

							while (scanned_bytes < max_scan) {
								decoder::Instruction next_inst;
								decoder::Decoder decoder_instance;
								if (!decoder_instance.decode(buffer.data() + next_offset + scanned_bytes, max_scan - scanned_bytes, next_inst))
									break;

								if (next_inst.mnemonic == "JMP" && next_inst.operands.size() == 1) {
									auto& jmp_op = next_inst.operands[0];
									if (jmp_op.type == decoder::OperandType::REG && jmp_op.reg == "rax") {
										uintptr_t target_addr = static_cast<uintptr_t>(inst.operands[1].immediate);
										local_detected.push_back({ start + i, target_addr, L"Detours-style MOV RAX; JMP RAX" });
										is_hook = true;
										break;
									}
								}
								scanned_bytes += next_inst.length;
							}
						}
					}

					// 3. Range heuristics for suspicious targets outside module
					if (!is_hook && hook_target != 0) {
						auto it_start = std::lower_bound(mod_starts.begin(), mod_starts.end(), hook_target);
						if (it_start == mod_starts.end() || hook_target < module_base || hook_target > module_base + section.virtual_size) {
							local_detected.push_back({ start + i, hook_target, L"Suspicious hook target outside module" });
							is_hook = true;
						}
					}

					if (!is_hook) {
						// No fallback here — fallback replaced by decoded instruction detection (already done)
						i += inst.length;
						continue;
					}

					i += inst.length;
				}
				else {
					i++;
				}
			}
			else {
				i++;
			}
		}

		// Merge local results into shared vector
		{
			std::lock_guard lock(detected_hooks_mutex);
			detected_hooks.insert(detected_hooks.end(),
				std::make_move_iterator(local_detected.begin()),
				std::make_move_iterator(local_detected.end()));
		}
	}

	std::vector<hook_detection> check_for_hooks(DWORD pid, const section_info& section, uintptr_t module_base, const std::vector<uintptr_t>& mod_starts, const std::vector<uintptr_t>& mod_ends)
	{
		std::vector<hook_detection> detected_hooks;
		std::mutex detected_hooks_mutex;

		auto h_process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
		if (!h_process) return detected_hooks;

		const uintptr_t text_mem_addr = module_base + section.base_address;

		size_t thread_count = std::thread::hardware_concurrency();
		if (thread_count == 0) {
			thread_count = 1;
		}

		size_t chunk_size = section.virtual_size / thread_count;
		std::vector<std::thread> workers;

		for (size_t t = 0; t < thread_count; ++t) {
			size_t start = t * chunk_size;
			size_t end = (t == thread_count - 1) ? section.virtual_size : start + chunk_size;

			workers.emplace_back(scan_section_for_hooks_thread,
				h_process, text_mem_addr, std::ref(section), module_base,
				std::cref(mod_starts), std::cref(mod_ends),
				start, end, std::ref(detected_hooks), std::ref(detected_hooks_mutex));
		}

		for (auto& w : workers)
			w.join();

		CloseHandle(h_process);
		return detected_hooks;
	}


} // namespace hook_checker

