#pragma once

// Core Headers

// C++ Standard Library Headers
#include <cstddef>
#include <cstdint>

// External Vendor Library Headers

constexpr uint64_t NULL_GUID = UINT64_MAX;

// Placeholders are used for various reasons through the engine. Meaning is
// placed upon the placeholder by the system/function that utilises it. For
// instance, something may not be NULL_GUID, but should not occupy a valid GUID,
// so it occupies a PLACEHOLDER_**_GUID so the engine knows how to handle it. It
// is more of a state setting than a GUID setting.
enum PlaceHolderGUID : uint64_t
{
    PLACEHOLDER_0_GUID = UINT64_MAX - 1,
    PLACEHOLDER_1_GUID = UINT64_MAX - 2,
    PLACEHOLDER_2_GUID = UINT64_MAX - 3,
    PLACEHOLDER_3_GUID = UINT64_MAX - 4,
    PLACEHOLDER_4_GUID = UINT64_MAX - 5,
    PLACEHOLDER_5_GUID = UINT64_MAX - 6,
    PLACEHOLDER_6_GUID = UINT64_MAX - 7,
    PLACEHOLDER_7_GUID = UINT64_MAX - 8,
    PLACEHOLDER_8_GUID = UINT64_MAX - 9,
    PLACEHOLDER_9_GUID = UINT64_MAX - 10
};

namespace BC
{

    class GUID
    {

	public:

		GUID();
		GUID(uint64_t uuid);
		GUID(const GUID&) = default;

		uint64_t GetGUID() const { return m_GUID; }
		operator uint64_t() const { return m_GUID; }

	private:

		uint64_t m_GUID;
		uint64_t GenerateGUID();
    };

}

namespace std 
{

	template <typename T> struct hash;

	template<>
	struct hash<BC::GUID>
	{
		std::size_t operator()(const BC::GUID& uuid) const
		{
			return uuid;
		}
	};
}