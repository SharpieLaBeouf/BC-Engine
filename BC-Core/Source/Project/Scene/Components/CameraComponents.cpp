#include "BC_PCH.h"
#include "CameraComponents.h"

#include "Asset/AssetManagerAPI.h"

#include <yaml-cpp/yaml.h>

namespace BC
{

    bool CameraComponent::Init()
    {
        // TODO: Implement -> Create Runtime Render Target Asset

        AssetHandle render_target_handle = Util::HashStringInsensitive("RT_ASSET_CAM_TAR - " + std::to_string(GetEntity().GetGUID()));

        return false;
    }

    bool CameraComponent::Shutdown()
    {
        // TODO: Implement -> Create Runtime Render Target Asset

        AssetHandle render_target_handle = Util::HashStringInsensitive("RT_ASSET_CAM_TAR - " + std::to_string(GetEntity().GetGUID()));
        AssetManager::RemoveRuntimeAsset(render_target_handle);
        
        return false;
    }

    CameraComponent::CameraComponent(const CameraComponent& other)
    {
        m_Instance          = std::make_unique<SceneCamera>(static_cast<const SceneCamera&>(*other.m_Instance));
        m_ClearType         = other.m_ClearType;
        m_ClearColour       = other.m_ClearColour;
        m_CameraViewport    = other.m_CameraViewport;
        m_ShouldDisplay     = other.m_ShouldDisplay;
        m_ForceRender       = other.m_ForceRender;
        m_CameraDepth       = other.m_CameraDepth;
        m_RenderTarget      = other.m_RenderTarget;
    }

    CameraComponent::CameraComponent(CameraComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Instance          = std::move(other.m_Instance);  other.m_Instance.reset();
        m_ClearType         = other.m_ClearType;            other.m_ClearType       = CameraClearType_Colour;
        m_ClearColour       = other.m_ClearColour;          other.m_ClearColour     = { 0.1764f, 0.3294f, 0.5607f, 1.0f };
        m_CameraViewport    = other.m_CameraViewport;       other.m_CameraViewport  = { 0.0f, 0.0f, 1.0f, 1.0f };
        m_ShouldDisplay     = other.m_ShouldDisplay;        other.m_ShouldDisplay   = true;
        m_ForceRender       = other.m_ForceRender;          other.m_ForceRender     = false;
        m_CameraDepth       = other.m_CameraDepth;          other.m_CameraDepth     = 0;
        m_RenderTarget      = other.m_RenderTarget;         other.m_RenderTarget    = NULL_GUID;
    }

    CameraComponent& CameraComponent::operator=(const CameraComponent& other)
    {
        if (this == &other)
            return *this;
        
        m_Instance          = std::make_unique<SceneCamera>(static_cast<const SceneCamera&>(*other.m_Instance));
        m_ClearType         = other.m_ClearType;
        m_ClearColour       = other.m_ClearColour;
        m_CameraViewport    = other.m_CameraViewport;
        m_ShouldDisplay     = other.m_ShouldDisplay;
        m_ForceRender       = other.m_ForceRender;
        m_CameraDepth       = other.m_CameraDepth;
        m_RenderTarget      = other.m_RenderTarget;

        return *this;
    }

    CameraComponent& CameraComponent::operator=(CameraComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Instance          = std::move(other.m_Instance);  other.m_Instance.reset();
        m_ClearType         = other.m_ClearType;            other.m_ClearType       = CameraClearType_Colour;
        m_ClearColour       = other.m_ClearColour;          other.m_ClearColour     = { 0.1764f, 0.3294f, 0.5607f, 1.0f };
        m_CameraViewport    = other.m_CameraViewport;       other.m_CameraViewport  = { 0.0f, 0.0f, 1.0f, 1.0f };
        m_ShouldDisplay     = other.m_ShouldDisplay;        other.m_ShouldDisplay   = true;
        m_ForceRender       = other.m_ForceRender;          other.m_ForceRender     = false;
        m_CameraDepth       = other.m_CameraDepth;          other.m_CameraDepth     = 0;
        m_RenderTarget      = other.m_RenderTarget;         other.m_RenderTarget    = NULL_GUID;

        return *this;
    }

    void CameraComponent::SetViewport(const glm::vec4& normalised_viewport, const glm::uvec2& render_target_size)
    {
        // TODO: Implement
    }
    
    void CameraComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool CameraComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }
}