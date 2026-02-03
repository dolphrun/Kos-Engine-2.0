#pragma once

#include <string>
#include <random>

namespace utility{
    struct GUID
    {
        uint64_t high = 0;
        uint64_t low = 0;

        void SetFromString(const std::string& str)
        {
            if(str.empty()) {
                high = 0;
                low = 0;
                return;
			}

            if (str.size() != 36)
                throw std::invalid_argument("Invalid GUID format");

            auto hex2int = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                throw std::invalid_argument("Invalid hex digit");
                };

            uint8_t bytes[16];
            int b = 0;

            for (size_t i = 0; i < str.size(); )
            {
                if (str[i] == '-') { ++i; continue; }

                bytes[b++] = (hex2int(str[i]) << 4) | hex2int(str[i + 1]);
                i += 2;
            }

            // Combine into high and low 64-bit integers
            high = ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) |
                ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) |
                ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |
                ((uint64_t)bytes[6] << 8) | ((uint64_t)bytes[7]);

            low = ((uint64_t)bytes[8] << 56) | ((uint64_t)bytes[9] << 48) |
                ((uint64_t)bytes[10] << 40) | ((uint64_t)bytes[11] << 32) |
                ((uint64_t)bytes[12] << 24) | ((uint64_t)bytes[13] << 16) |
                ((uint64_t)bytes[14] << 8) | ((uint64_t)bytes[15]);
        }

        std::string GetToString() const
        {
            char buf[37];
            snprintf(buf, sizeof(buf),
                "%08llx-%04llx-%04llx-%04llx-%02llx%02llx%02llx%02llx%02llx%02llx",
                (high >> 32) & 0xFFFFFFFFULL,
                (high >> 16) & 0xFFFFULL,
                high & 0xFFFFULL,
                (low >> 48) & 0xFFFFULL,
                (low >> 40) & 0xFFULL,
                (low >> 32) & 0xFFULL,
                (low >> 24) & 0xFFULL,
                (low >> 16) & 0xFFULL,
                (low >> 8) & 0xFFULL,
                low & 0xFFULL);
            return std::string(buf);
        }


        bool Empty() const {
            return (high == 0 && low == 0);
        }


        bool operator<(const GUID& other) const noexcept
        {
            return (high < other.high) ||
                (high == other.high && low < other.low);
        }

        bool operator==(const GUID& other) const noexcept
        {
            return (high == other.high) && (low == other.low);
        }

        void operator()() = delete;
    };

    inline GUID GenerateGUID()
    {
        GUID guid;
        static std::random_device dev;
        static std::mt19937 rng(dev());

        std::uniform_int_distribution<int> dist(0, 15);

        const char* v = "0123456789abcdef";
        const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

        std::string res;
        for (int i = 0; i < 16; i++) {
            if (dash[i]) res += "-";
            res += v[dist(rng)];
            res += v[dist(rng)];
        }

        guid.SetFromString(res);

        return guid;
    }
}

namespace std {
    template<>
    struct hash<utility::GUID> {
        size_t operator()(const utility::GUID& g) const noexcept {
            return (size_t)(g.high ^ (g.low * 0x9e3779b97f4a7c15ULL));
        }
    };
}