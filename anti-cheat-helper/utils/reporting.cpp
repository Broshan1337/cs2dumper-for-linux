#include "reporting.h"
#include <iomanip>
#include <sstream>

namespace utils {

    std::mutex log_mutex;
    std::vector<std::wstring> log_lines;

    static std::wstring to_wstring(const std::string& s) {
        return std::wstring(s.begin(), s.end());
    }

    std::wstring string_to_wstring(const std::string& str) {
        if (str.empty()) return {};
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
    }

    void report_hook(const std::wstring& module, const std::wstring& type, uintptr_t offset, uintptr_t target) {
        std::wostringstream oss;
        oss << L"[HOOK DETECTED] Module: " << module
            << L", Type: " << type
            << L", Offset: 0x" << std::hex << offset
            << L", Target: 0x" << std::hex << target << L"\n";
        std::wcout << oss.str();
    }

    void report_exec_region(uintptr_t address, size_t size, const std::string& perms, const std::string& hex_snippet) {
        std::ostringstream oss;
        oss << "[EXEC REGION] Address: 0x" << std::hex << address
            << ", Size: " << std::dec << size
            << ", Perms: " << perms
            << ", Hex: " << hex_snippet << "\n";

        std::cout << oss.str();
    }

    void add_log(const char* fmt, ...) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        std::wstring wbuf(buf, buf + strlen(buf));
        wbuf += L"\n";

        std::lock_guard<std::mutex> lock(log_mutex);
        log_lines.push_back(wbuf);
        std::wcout << wbuf << std::flush;
    }

} // namespace utils
