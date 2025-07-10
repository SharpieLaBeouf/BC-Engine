#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{

    class Shader : public Asset
    {
        
    public:

        Shader() = default;

        AssetType GetType() const override { return m_Compute ? AssetType::Compute_Shader : AssetType::Shader; }
        void SetComputeShader() { m_Compute = true; }

        static std::shared_ptr<Shader> CreateShader(const std::string& shader_source);
        static std::shared_ptr<Shader> CreateComputeShader(const std::string& shader_source);

    private:
        
        bool m_Compute = false;

    };

}