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

#include "../decoder/decoder.h" 
#include "hook_detector.h"
#include "../utils/module_utils.h"


namespace vac3 {

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

	void scan_section_for_hooks_thread(
		HANDLE h_process,
		uintptr_t text_mem_addr,
		const section_info_t& section,
		uintptr_t module_base,
		const std::vector<uintptr_t>& mod_starts,
		const std::vector<uintptr_t>& mod_ends,
		size_t start,
		size_t end,
		std::vector<hook_detection_t>& detected_hooks,
		std::mutex& detected_hooks_mutex)
	{
		std::vector<hook_detection_t> local_detected;

		std::vector<uint8_t> buffer(end - start);
		SIZE_T bytes_read = 0;

		if (!ReadProcessMemory(h_process, reinterpret_cast<LPCVOID>(text_mem_addr + start), buffer.data(), buffer.size(), &bytes_read))
			return;

		const size_t compare_size = std::min<size_t>(bytes_read, section.original_bytes.size() - start);

		for (size_t i = 0; i + 14 < compare_size;) {
			if (buffer[i] != section.original_bytes[start + i] && !utils::is_whitelisted_offset(start + i)) {
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
						// No fallback here ? fallback replaced by decoded instruction detection (already done)
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
			std::lock_guard<std::mutex> lock(detected_hooks_mutex);
			detected_hooks.insert(detected_hooks.end(),
				std::make_move_iterator(local_detected.begin()),
				std::make_move_iterator(local_detected.end()));
		}
	}

	bool load_text_section_snapshot(const std::wstring& exe_path, section_info_t& out_section, uintptr_t image_base) {
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

	static bool matches_mov_rax_jmp_rax_pattern(const uint8_t* ptr) {
		// mov rax, imm64 (48 B8 imm64) + jmp rax (FF E0)
		return (ptr[0] == 0x48 && ptr[1] == 0xB8 && ptr[10] == 0xFF && ptr[11] == 0xE0);
	}

	static bool is_in_relocations(size_t offset, const std::vector<relocation_block_t>& relocations) {
		for (const auto& block : relocations) {
			uintptr_t page_start = block.page_rva;
			for (uint16_t off : block.offsets) {
				if (page_start + off == offset)
					return true;
			}
		}
		return false;
	}

	bool address_in_any_module(uintptr_t addr, const std::vector<uintptr_t>& starts, const std::vector<uintptr_t>& ends) {
		for (size_t i = 0; i < starts.size(); ++i) {
			if (addr >= starts[i] && addr < ends[i])
				return true;
		}
		return false;
	}

	std::vector<hook_detection_t> check_for_hooks(DWORD pid, const section_info_t& section, uintptr_t module_base, const std::vector<uintptr_t>& mod_starts, const std::vector<uintptr_t>& mod_ends)
	{
		std::vector<hook_detection_t> detected_hooks;
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

} // namespace vac3