#pragma once

// Core Headers
#include "Component Base.h"

#include "Graphics/SceneCamera.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{
    
    struct CameraComponent : public ComponentBase
    {
    
    public:

        using CameraClearType = uint8_t;
        enum : CameraClearType
        {
            CameraClearType_Colour,
            CameraClearType_Skybox
        };

        CameraComponent() = default;
        ~CameraComponent() = default;

        CameraComponent(const CameraComponent& other);
        CameraComponent(CameraComponent&& other) noexcept;
        CameraComponent& operator=(const CameraComponent& other);
        CameraComponent& operator=(CameraComponent&& other) noexcept;

        bool Init() override;
        bool Shutdown() override;

        ComponentType GetType() override { return ComponentType::CameraComponent; }

        SceneCamera* GetCamera() const { return m_Instance.get(); }

        CameraClearType GetClearType() const { return m_ClearType; }
        void SetClearType(CameraClearType clear_type) { m_ClearType = clear_type; }

        const glm::vec4& GetClearColour() const { return m_ClearColour; }
        void SetClearColour(const glm::vec4& clear_colour) { m_ClearColour = clear_colour; }

        AssetHandle GetSkyboxHandle() const { return m_SkyboxHandle; }
        void SetSkyboxHandle(AssetHandle skybox_handle) { m_SkyboxHandle = skybox_handle; }

        const glm::vec4& GetViewport() const { return m_CameraViewport; }
        void SetViewport(const glm::vec4& normalised_viewport, const glm::uvec2& render_target_size);

        bool GetShouldDisplay() const { return m_ShouldDisplay; }
        void SetShouldDisplay(bool should_display) { m_ShouldDisplay = should_display; }

        bool GetForceRender() const { return m_ForceRender; }
        void SetForceRender(bool force_render) { m_ForceRender = force_render; }

        uint8_t GetDepth() const { return m_CameraDepth; }
        void SetDepth(uint8_t depth) { m_CameraDepth = depth; }

        AssetHandle GetRenderTarget() const { return m_RenderTarget; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        
        std::unique_ptr<SceneCamera> m_Instance = std::make_unique<SceneCamera>();

        /// @brief The type of clear to be performed when rendering, either colour, or a skybox
        CameraClearType m_ClearType = CameraClearType_Colour;

        /// @brief The colour to clear the image with prior to drawing
        glm::vec4 m_ClearColour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };

        /// @brief The AssetHandle for the skybox if clear type is skybox
        AssetHandle m_SkyboxHandle = NULL_GUID;

        /// @brief Camera instance tied to component
        /// X = Pos X
        /// Y = Pos Y
        /// Z = Width
        /// W = Height
        /// 
        /// All Values Are Normalised Between 0.0 -> 1.0f.
        glm::vec4 m_CameraViewport = { 0.0f, 0.0f, 1.0f, 1.0f };

        /// @brief Should this camera contribute to the final viewport
        bool m_ShouldDisplay = true;

        /// @brief If this camera is either not Displayed to the viewport, or is
        /// obfuscated by another Camera's final image, should we still force
        /// it to render? 
        ///
        /// E.g., if it is used for post-processing input, we may need
        /// this as input
        bool m_ForceRender = false;

        /// @brief The depth of the camera on the final viewport
        ///
        /// If this Camera has a lower depth than another Camera that entirely
        /// covers it, it will not be included for render.
        uint8_t m_CameraDepth = 0;

        /// @brief A runtime asset will be generated and stored in the
        /// AssetManager for the RenderTarget which will be referenced by this
        /// handle for this Camera.
        AssetHandle m_RenderTarget = NULL_GUID;
    };

}