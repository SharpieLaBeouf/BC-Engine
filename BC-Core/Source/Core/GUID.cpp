#include "BC_PCH.h"
#include "GUID.h"

// Core Headers

// C++ Standard Library Headers
#include <random>
#include <unordered_map>
#include <limits>

// External Vendor Library Headers

namespace BC {

	static std::random_device s_RandomDevice;
	static std::mt19937 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	GUID::GUID() : m_GUID(s_UniformDistribution(s_Engine)) { }
	GUID::GUID(uint64_t uuid) : m_GUID(uuid) { }

	uint64_t GUID::GenerateGUID() 
    {
		std::uniform_int_distribution<uint64_t> distribution(0, std::numeric_limits<uint64_t>::max() - 11);
		return distribution(s_Engine);
	}

}