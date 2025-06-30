#include "BC_PCH.h"
#include "CameraComponents.h"

#include "Asset/AssetManagerAPI.h"
#include "Asset/Assets/RenderTarget.h"

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

    void CameraComponent::SetViewport(const glm::vec4& normalised_viewport, const glm::uvec2& screen_space_render_target_size)
    {
        m_CameraViewport = normalised_viewport;

        // TODO: Implement - resize RenderTarget
        uint32_t true_width = static_cast<uint32_t>(std::max(1u, screen_space_render_target_size.x) * std::clamp(normalised_viewport.z, 0.0f, 1.0f));
        uint32_t true_height = static_cast<uint32_t>(std::max(1u, screen_space_render_target_size.y) * std::clamp(normalised_viewport.w, 0.0f, 1.0f));
    
        // Enforce atleast 1x1 pixels
        true_width = static_cast<uint32_t>(std::max(1u, true_width));
        true_height = static_cast<uint32_t>(std::max(1u, true_height));
        
        auto scene_manager = Application::GetProject()->GetSceneManager();
        if (scene_manager->IsRunning())
        {
            auto render_target = GetRenderTarget();
            BC_ASSERT(render_target, "CameraComponent::SetViewport: Could Not Set Viewport - Cannot Get Render Target Runtime Asset.");
            if (!render_target) // Log as error if release
            {
                BC_CORE_FATAL("CameraComponent::SetViewport: Could Not Set Viewport - Cannot Get Render Target Runtime Asset.");
                return;
            }
            render_target->Resize(true_width, true_height);
        }
    }

    RenderTarget* CameraComponent::GetRenderTarget()
    {
        if (!AssetManager::IsAssetLoaded(m_RenderTarget))
            Init();
        
        auto render_target = AssetManager::GetAsset<RenderTarget>(m_RenderTarget);
        return render_target.get();
    }

    void CameraComponent::SceneSerialise(YAML::Emitter &out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            out << YAML::Key << "Clear Type" << YAML::Value << (m_ClearType == CameraClearType_Colour ? "Colour" : "Skybox");
            
            out << YAML::Key << "Clear Colour" << YAML::Value << YAML::Flow;
            out << YAML::BeginSeq;
            {
                out << YAML::Value << m_ClearColour.r;
                out << YAML::Value << m_ClearColour.g;
                out << YAML::Value << m_ClearColour.b;
                out << YAML::Value << m_ClearColour.a;
            }
            out << YAML::EndSeq;
            
            out << YAML::Key << "Skybox Handle" << YAML::Value << (uint64_t)m_SkyboxHandle;
            
            out << YAML::Key << "Viewport" << YAML::Value << YAML::Flow;
            out << YAML::BeginSeq;
            {
                out << YAML::Value << m_CameraViewport.x;
                out << YAML::Value << m_CameraViewport.y;
                out << YAML::Value << m_CameraViewport.z;
                out << YAML::Value << m_CameraViewport.w;
            }
            out << YAML::EndSeq;
            
            out << YAML::Key << "Should Display" << YAML::Value << m_ShouldDisplay;
            out << YAML::Key << "Force Render" << YAML::Value << m_ForceRender;
            out << YAML::Key << "Camera Depth" << YAML::Value << static_cast<int>(m_CameraDepth);
            
            out << YAML::Key << "Camera Instance" << YAML::Value;
            out << YAML::BeginMap;
            {
                SceneCamera camera;
                if (m_Instance)
                {
                    camera = *m_Instance;
                }
                else
                {
                    camera = SceneCamera();
                }

                if (camera.GetType() == SceneCamera::CameraProjType_Perspective)
                {
                    out << YAML::Key << "Type" << YAML::Value << "Perspective";
                    out << YAML::Key << "FOV" << YAML::Value << camera.GetPerspectiveFOV();
                    out << YAML::Key << "Near" << YAML::Value << camera.GetPerspectiveFOV();
                    out << YAML::Key << "Far" << YAML::Value << camera.GetPerspectiveFOV();
                    
                }
                else
                {
                    out << YAML::Key << "Type" << YAML::Value << "Orthographic";
                    out << YAML::Key << "Size" << YAML::Value << camera.GetOrthographicSize();
                    out << YAML::Key << "Near" << YAML::Value << camera.GetOrthographicNearClip();
                    out << YAML::Key << "Far" << YAML::Value << camera.GetOrthographicFarClip();
                }
            }
            out << YAML::EndMap;
        }
		out << YAML::EndMap;
    }

    bool CameraComponent::SceneDeserialise(const YAML::Node& data)
    {
        if (data["Clear Type"])
            m_ClearType = data["Clear Type"].as<std::string>() == "Skybox" ? CameraClearType_Skybox : CameraClearType_Colour;

        for (int i = 0; i < 4 && data["Clear Colour"] && data["Clear Colour"].size() == 4; ++i)
            m_ClearColour[i] = data["Clear Colour"][i].as<float>();

        if (data["Skybox Handle"])
            m_SkyboxHandle = data["Skybox Handle"].as<uint64_t>();

        for (int i = 0; i < 4 && data["Viewport"] && data["Viewport"].size() == 4; ++i)
            m_CameraViewport[i] = data["Viewport"][i].as<float>();
        
        if (data["Should Display"])
            m_ShouldDisplay = data["Should Display"].as<bool>();
            
        if (data["Force Render"])
            m_ForceRender = data["Force Render"].as<bool>();
            
        if (data["Camera Depth"])
            m_CameraDepth = data["Camera Depth"].as<uint8_t>();

        if (auto camera_data = data["Camera Instance"]; camera_data)
        {
            SceneCamera::CameraProjType type;
            if (camera_data["Type"])
                type = camera_data["Type"].as<std::string>() == "Orthographic" ? SceneCamera::CameraProjType_Orthographic : SceneCamera::CameraProjType_Perspective;
            else
                return false;
            
            if (!m_Instance)
                m_Instance = std::make_unique<SceneCamera>();

            switch (type)
            {
                case SceneCamera::CameraProjType_Perspective:
                {
                    m_Instance->SetPerspective(
                        camera_data["FOV"].as<float>(), 
                        camera_data["Near"].as<float>(), 
                        camera_data["Far"].as<float>());
                    break;
                }
                case SceneCamera::CameraProjType_Orthographic:
                {
                    m_Instance->SetOrthographic(
                        camera_data["Size"].as<float>(), 
                        camera_data["Near"].as<float>(), 
                        camera_data["Far"].as<float>());
                    break;
                }
                default: return false;
            }
        }
            
        return true;
    }
}