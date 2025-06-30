#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace BC
{
    using StringHash = uint64_t;
}

namespace BC::Util
{

    // fnv1a_hash
    constexpr uint64_t HashString(const char* str)
    {
        uint64_t hash = 14695981039346656037ull; 
        while (*str)
        {
            hash ^= static_cast<uint64_t>(*str++);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    // fnv1a_hash
    constexpr uint64_t HashString(std::string_view str)
    {
        uint64_t hash = 14695981039346656037ull;
        for (char c : str)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    // fnv1a_hash
    inline uint64_t HashString(const std::string& str)
    {
        return HashString(std::string_view{str});
    }

    // fnv1a_hash
    constexpr uint64_t HashBytes(const void* data, size_t length)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        uint64_t hash = 14695981039346656037ull;

        for (size_t i = 0; i < length; ++i)
        {
            hash ^= static_cast<uint64_t>(bytes[i]);
            hash *= 1099511628211ull;
        }

        return hash;
    }

    // fnv1a_hash
    constexpr uint64_t HashStringInsensitive(const char* str)
    {
        uint64_t hash = 14695981039346656037ull;
        while (*str)
        {
            char c = *str++;
            if (c >= 'A' && c <= 'Z')
                c = c + 32;
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    // fnv1a_hash
    constexpr uint64_t HashStringInsensitive(std::string_view str)
    {
        uint64_t hash = 14695981039346656037ull;
        for (char c : str)
        {
            if (c >= 'A' && c <= 'Z')
                c += 32;
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    inline uint64_t HashStringInsensitive(const std::string& str)
    {
        return HashStringInsensitive(std::string_view{str});
    }

    constexpr uint64_t HashCombine(uint64_t a, uint64_t b)
    {
        a ^= b + 0x9e3779b97f4a7c15ull + (a << 12) + (a >> 4);
        return a;
    }
}