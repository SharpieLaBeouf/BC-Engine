#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers


namespace YAML
{
    class Emitter;
    class Node;
}

namespace BC
{
    using MaterialRenderType = uint8_t;
	enum : MaterialRenderType 
    {
        MaterialRenderType_Opaque,
        MaterialRenderType_Mixed,
        MaterialRenderType_Transparent
	};

    class Material : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Material; }

        MaterialRenderType GetRenderType() const { return m_RenderType; }
        void SetRenderType(MaterialRenderType render_type) { m_RenderType = render_type; }

        const glm::vec4& GetAlbedoColour() const { return m_AlbedoTint; }
        void SetAlbedoColour(const glm::vec4& colour) { m_AlbedoTint = colour; }

        float GetAlpha() const { return m_Alpha; }
        void SetAlpha(float alpha)
        {
            m_Alpha = alpha;
            if (m_Alpha < 1.0f && m_RenderType == MaterialRenderType_Opaque)
            {
                m_RenderType = MaterialRenderType_Transparent;
            }
        }
        
        float GetMetallic() const { return m_Metallic; }
        void SetMetallic(float metallic) { m_Metallic = metallic; }
        
        float GetRoughness() const { return m_Roughness; }
        void SetRoughness(float roughness) { m_Roughness = roughness; }

        AssetHandle GetAlbedoTexture() const { return m_AlbedoTexture; }
        void SetAlbedoTexture(AssetHandle albedo_handle) { m_AlbedoTexture = albedo_handle; }

        AssetHandle GetMetallicTexture() const { return m_MetallicTexture; }
        void SetMetallicTexture(AssetHandle metallic_handle) { m_MetallicTexture = metallic_handle; }

        AssetHandle GetNormalTexture() const { return m_NormalTexture; }
        void SetNormalTexture(AssetHandle normal_handle) { m_NormalTexture = normal_handle; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }

        void DeserialiseAsset(const YAML::Node& data)
        {

        }

        static std::shared_ptr<Material> CreateMaterial(const std::string& serialised_material_data);

    private:

        glm::vec4 m_AlbedoTint = { 1.0f, 1.0f, 1.0f, 1.0f };
        float m_Alpha = 1.0f;
        float m_Roughness = 0.7f;
        float m_Metallic = 0.7f;

        AssetHandle m_AlbedoTexture = NULL_GUID;
        AssetHandle m_MetallicTexture = NULL_GUID;
        AssetHandle m_NormalTexture = NULL_GUID;

        MaterialRenderType m_RenderType;

    };

    class Skybox : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Skybox; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }
        
        void DeserialiseAsset(const YAML::Node& data)
        {

        }

        static std::shared_ptr<Skybox> CreateSkybox(const std::string& serialised_skybox_data);

    private:

        

    };

}