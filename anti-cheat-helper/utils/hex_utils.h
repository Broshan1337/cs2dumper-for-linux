#pragma once
#include <string>
#include <vector>

#include <sstream>
#include <iomanip>

namespace utils {

    inline std::string bytes_to_hex(const uint8_t* data, size_t length) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            oss << std::setw(2) << static_cast<int>(data[i]);
            if (i + 1 != length)
                oss << ' ';
        }
        return oss.str();
    }

   inline std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
        return bytes_to_hex(bytes.data(), bytes.size());
   }

} // namespace utils