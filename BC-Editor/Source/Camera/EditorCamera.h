#pragma once

#include "Asset/Assets/RenderTarget.h"
#include "Project/Scene/Bounds/Frustum.h"

#include <memory>

#include <glm/glm.hpp>

namespace BC
{
    class EditorCamera
    {

    public:

        using CameraProjType = uint8_t;
        enum : CameraProjType
        {
            CameraProjType_Perspective,
            CameraProjType_Orthographic
        };

    public:

        EditorCamera() = delete;
        EditorCamera(uint32_t width, uint32_t height);
        ~EditorCamera() = default;

        EditorCamera(const EditorCamera&) = default;
        EditorCamera(EditorCamera&&) = default;
        EditorCamera& operator=(const EditorCamera&) = default;
        EditorCamera& operator=(EditorCamera&&) = default;

        // ---- General Methods ----
        void OnUpdate();

        // ---- General Setters ----
        void SetViewportSize(uint32_t width, uint32_t height);
        float GetAspectRatio() const { return m_AspectRatio; }

		void SetNear(float near_plane)    { m_Near = near_plane; }
		void SetFar(float far_plane)      { m_Far = far_plane;   }

        // ---- General Getters ----
        CameraProjType GetType() const { return m_Type; }

        RenderTarget* GetRenderTarget() const { return m_RenderTarget.get(); }

		float GetNear()  const { return m_Near; }
		float GetFar()   const { return m_Far;  }
        
		const glm::mat4& GetProjMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

        const glm::vec3& GetPosition() const;
        glm::quat GetOrientation() const;

        const Frustum& GetFrustum() const;

        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;

        // ---- Perspective Getters & Setters ----
		void SetPerspective(float fov = 60.0f, float near_clip = 0.01f, float far_clip = 1000.0f);
        
		float GetPerspectiveFOV()           const { return m_PerspectiveFOV;  }
        
		void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
        
        // ---- Orthographic Getters & Setters ----
		void SetOrthographic(float size = 10.0f, float near_clip = -1.0f, float far_clip = 1.0f);

		float GetOrthographicSize()         const { return m_OrthographicSize; }

		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }

    private:

        void RecalculateView();
		void RecalculateProjection();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		glm::vec2 PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
        
    private:

        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);

        glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_FocalPoint = glm::vec3(0.0f);

		float m_Distance = 10.0f;
        float m_Pitch = glm::radians(20.0f), m_Yaw = glm::radians(180.0f);

        glm::uvec2 m_ViewportSize = glm::uvec2(1, 1);

        Frustum m_Frustum = {};

        float m_Near    = 0.01f;
        float m_Far     = 1000.0f;
        
		float m_AspectRatio         = 1.0f;
		float m_PerspectiveFOV      = 60.0f; // Degrees
		float m_OrthographicSize    = 10.0f;

        std::shared_ptr<RenderTarget> m_RenderTarget;

        CameraProjType m_Type = CameraProjType_Perspective;

    };
}