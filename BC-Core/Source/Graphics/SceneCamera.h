#pragma once

#include "Project/Scene/Bounds/Frustum.h"

#include <cstdint>

#include <glm/glm.hpp>

namespace BC
{

    class SceneCamera
    {

    public:

        using CameraProjType = uint8_t;
        enum : CameraProjType
        {
            CameraProjType_Perspective,
            CameraProjType_Orthographic
        };

    public:

        SceneCamera() { RecalculateProjection(); }
        ~SceneCamera() = default;

        SceneCamera(const SceneCamera&) = default;
        SceneCamera(SceneCamera&&) = default;
        SceneCamera& operator=(const SceneCamera&) = default;
        SceneCamera& operator=(SceneCamera&&) = default;

        CameraProjType GetType() const { return m_Type; }

		const glm::mat4& GetProjMatrix() const { return m_Projection; } // View matrix is glm::inverse(entity_global_transform_matrix); not handled by SceneCamera

        void SetViewportSize(uint32_t width, uint32_t height);

        float GetAspectRatio() const { return m_AspectRatio; }

        // ---- Perspective ----
		void SetPerspective(float fov = 60.0f, float near_clip = 0.01f, float far_clip = 1000.0f);
        
		float GetPerspectiveFOV()           const { return m_PerspectiveFOV;  }
		float GetPerspectiveNearClip()      const { return m_PerspectiveNear; }
		float GetPerspectiveFarClip()       const { return m_PerspectiveFar;  }
        
		void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
		void SetPerspectiveNearClip(float near_clip) { m_PerspectiveNear = near_clip; RecalculateProjection(); }
		void SetPerspectiveFarClip(float far_clip) { m_PerspectiveFar = far_clip; RecalculateProjection(); }
        
        // ---- Orthographic ----
		void SetOrthographic(float size = 10.0f, float near_clip = -1.0f, float far_clip = 1.0f);

		float GetOrthographicSize()         const { return m_OrthographicSize; }
		float GetOrthographicNearClip()     const { return m_OrthographicNear; }
		float GetOrthographicFarClip()      const { return m_OrthographicFar;  }

		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		void SetOrthographicNearClip(float near_clip) { m_OrthographicNear = near_clip; RecalculateProjection(); }
		void SetOrthographicFarClip(float far_clip) { m_OrthographicFar = far_clip; RecalculateProjection(); }

	private:

		void RecalculateProjection();

    private:

		glm::mat4 m_Projection = glm::mat4(1.0f);

        Frustum m_Frustum = {};

		float m_PerspectiveFOV = 60.0f; // Degrees
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 1.0f;

        CameraProjType m_Type = CameraProjType_Perspective;

    };

}