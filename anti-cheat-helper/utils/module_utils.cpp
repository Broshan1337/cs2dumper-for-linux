// utils/module_utils.cpp
#include "module_utils.h"
#include <psapi.h>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <TlHelp32.h>

namespace utils {

	static std::mutex g_whitelist_mutex;
	static std::vector<size_t> g_whitelist_offsets;

	bool enum_process_modules(DWORD pid, std::vector<HMODULE>& out_modules) {
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (!hProcess)
			return false;

		DWORD needed = 0;
		out_modules.resize(1024);
		if (!EnumProcessModulesEx(hProcess, out_modules.data(), static_cast<DWORD>(out_modules.size() * sizeof(HMODULE)), &needed, LIST_MODULES_ALL)) {
			CloseHandle(hProcess);
			return false;
		}
		size_t count = needed / sizeof(HMODULE);
		out_modules.resize(count);

		CloseHandle(hProcess);
		return true;
	}

	bool get_module_info(HMODULE hMod, uintptr_t& base, size_t& size, std::wstring& mod_name) {
		WCHAR name[MAX_PATH]{};
		if (!GetModuleFileNameExW(GetCurrentProcess(), hMod, name, MAX_PATH))
			return false;

		MODULEINFO modinfo{};
		if (!GetModuleInformation(GetCurrentProcess(), hMod, &modinfo, sizeof(modinfo)))
			return false;

		base = reinterpret_cast<uintptr_t>(modinfo.lpBaseOfDll);
		size = static_cast<size_t>(modinfo.SizeOfImage);

		mod_name = name;
		return true;
	}

	void get_module_ranges(DWORD pid, std::vector<uintptr_t>& out_starts, std::vector<uintptr_t>& out_ends, std::vector<std::wstring>& out_names) {
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if (!hProcess)
			return;

		HMODULE modules[1024];
		DWORD cbNeeded;

		if (EnumProcessModulesEx(hProcess, modules, sizeof(modules), &cbNeeded, LIST_MODULES_ALL)) {
			size_t count = cbNeeded / sizeof(HMODULE);
			for (size_t i = 0; i < count; ++i) {
				MODULEINFO modinfo{};
				WCHAR mod_name[MAX_PATH]{};
				if (GetModuleInformation(hProcess, modules[i], &modinfo, sizeof(modinfo)) &&
					GetModuleBaseNameW(hProcess, modules[i], mod_name, MAX_PATH)) {
					out_starts.push_back(reinterpret_cast<uintptr_t>(modinfo.lpBaseOfDll));
					out_ends.push_back(reinterpret_cast<uintptr_t>(modinfo.lpBaseOfDll) + modinfo.SizeOfImage);
					out_names.emplace_back(mod_name);
				}
			}
		}

		CloseHandle(hProcess);
	}

	std::wstring to_lower(const std::wstring& s) {
		std::wstring out = s;
		std::transform(out.begin(), out.end(), out.begin(), towlower);
		return out;
	}

	bool get_process_exe_path(DWORD pid, std::wstring& out_path) {
		auto h_process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		
		if (!h_process) 
			return false;

		wchar_t buffer[MAX_PATH]{};
		DWORD size = MAX_PATH;
		auto success = QueryFullProcessImageNameW(h_process, 0, buffer, &size) == TRUE;
		if (success) out_path.assign(buffer, size);

		CloseHandle(h_process);
		return success;
	}

	DWORD get_process_id_by_name(const wchar_t* process_name) {
		DWORD pid = 0;
		PROCESSENTRY32W entry{};
		entry.dwSize = sizeof(entry);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
			return 0;

		if (Process32FirstW(snapshot, &entry)) {
			do {
				if (_wcsicmp(entry.szExeFile, process_name) == 0) {
					pid = entry.th32ProcessID;
					break;
				}
			} while (Process32NextW(snapshot, &entry));
		}

		CloseHandle(snapshot);
		return pid;
	}

	// Checks if mod_name matches any trusted DLL name (case-insensitive substring match)
	bool is_trusted_module(const std::wstring& mod_name) {
		static const std::set<std::wstring> trusted = {
			L"ntdll.dll", L"kernel32.dll", L"kernelbase.dll", L"user32.dll",
			L"gdi32.dll", L"win32u.dll", L"advapi32.dll", L"msvcrt.dll",
			L"mscoree.dll", L"clrjit.dll", L"clr.dll", L"shell32.dll",
			L"shcore.dll", L"combase.dll", L"rpcrt4.dll", L"ole32.dll",
			L"oleaut32.dll", L"crypt32.dll", L"secur32.dll", L"ws2_32.dll",
			L"wininet.dll", L"dbghelp.dll", L"psapi.dll", L"ntmarta.dll",
			L"uxtheme.dll", L"d3d11.dll", L"d3d9.dll", L"dwmapi.dll",
			L"dxgi.dll", L"dxcore.dll", L"msimg32.dll", L"kernel.appcore.dll",
			L"powrprof.dll", L"profapi.dll", L"sechost.dll", L"shlwapi.dll",
			L"version.dll", L"winmm.dll", L"wow64.dll", L"wow64win.dll",
			L"wow64cpu.dll", L"bcryptprimitives.dll", L"bcrypt.dll",

			// Additional common system DLLs
			L"kernel32full.dll", L"userenv.dll", L"mswsock.dll", L"ncrypt.dll",
			L"cfgmgr32.dll", L"dbgcore.dll", L"iphlpapi.dll", L"wsock32.dll",
			L"dinput8.dll", L"hid.dll", L"msvcp_win.dll", L"wintrust.dll",
			L"imagehlp.dll", L"msvcp140.dll", L"vcruntime140.dll",
			L"api-ms-win-core-synch-l1-2-0.dll", L"api-ms-win-core-file-l2-1-0.dll",
			L"api-ms-win-core-processthreads-l1-1-2.dll", L"api-ms-win-core-memory-l1-1-0.dll",
			L"comdlg32.dll", L"dcomp.dll", L"cryptbase.dll", L"windows.storage.dll",
			L"windows.ui.dll",

			// Graphics related
			L"d3d12.dll", L"d3dcompiler_47.dll", L"d2d1.dll", L"dwrite.dll",
			L"dxguid.dll", L"dxva2.dll", L"msvcp140_1.dll",

			// Networking and Security
			L"ncryptsslp.dll", L"cryptnet.dll", L"cryptsvc.dll", L"winhttp.dll",
			L"wshtcpip.dll",

			// Multimedia and sound
			L"avrt.dll", L"mmdevapi.dll", L"audioses.dll",

			// Windows Runtime & UWP
			L"windows.ui.xaml.dll", L"windows.ui.input.dll", L"windows.foundation.dll",
			L"windows.storage.streams.dll", L"windows.data.xml.dom.dll"
		};

		std::wstring lower_name = to_lower(mod_name);

		for (const auto& trusted_name : trusted) {
			if (lower_name.find(trusted_name) != std::wstring::npos)
				return true;
		}
		return false;
	}

	void add_whitelist_offset(size_t offset) {
		std::lock_guard<std::mutex> lock(g_whitelist_mutex);
		g_whitelist_offsets.push_back(offset);
	}

	bool is_whitelisted_offset(size_t offset) {
		std::lock_guard<std::mutex> lock(g_whitelist_mutex);
		return std::find(g_whitelist_offsets.begin(), g_whitelist_offsets.end(), offset) != g_whitelist_offsets.end();
	}

	bool is_game_module(const std::wstring& mod_name) {
		static const std::wstring game_modules[] = {
			L"cs2.exe",
			L"engine2.dll",
			L"client.dll",
			L"rendersystemdx11.dll",
			L"materialsystem2.dll",
			L"inputsystem.dll",
			L"vphysics2.dll",
			L"soundsystem.dll",
			L"tier0.dll",
			L"vstdlib.dll",
			L"filesystem_stdio2.dll",
			L"steamnetworkingsockets.dll",
			L"matchmaking.dll",
			L"datacache.dll",
			L"studiorender.dll",
			L"shaderapidx11.dll"
		};

		for (const auto& gm : game_modules) {
			if (_wcsicmp(gm.c_str(), mod_name.c_str()) == 0)
				return true;
		}
		return false;
	}

} // namespace utils
