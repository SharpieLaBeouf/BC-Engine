#include "BC_PCH.h"
#include "SceneCamera.h"

namespace BC
{
    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
		BC_ASSERT(width > 0 && height > 0, "SceneCamera::SetViewportSize: Cannot Set Viewport Size to Less or Equal to 0.");
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
		RecalculateProjection();
    }

    void SceneCamera::SetPerspective(float fov, float near_clip, float far_clip)
    {
        m_Type = CameraProjType_Perspective;

        m_PerspectiveFOV    = std::clamp(fov, 1.0f, 179.0f);
        m_PerspectiveNear   = std::max(near_clip, 0.0001f);
        m_PerspectiveFar    = std::max(far_clip, m_PerspectiveNear + 0.001f);

		RecalculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float near_clip, float far_clip)
    {
        m_Type = CameraProjType_Orthographic;

        m_OrthographicSize = std::max(size, 0.0001f);
        m_OrthographicNear = near_clip;
        m_OrthographicFar = std::max(far_clip, near_clip + 0.001f);
        
		RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        if (m_Type == CameraProjType_Perspective)
        {
            m_Projection = glm::perspective(glm::radians(m_PerspectiveFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
        }
        else
        {
            float orthoLeft     = -m_OrthographicSize * m_AspectRatio * 0.5f;
            float orthoRight    =  m_OrthographicSize * m_AspectRatio * 0.5f;
            float orthoBottom   = -m_OrthographicSize * 0.5f;
            float orthoTop      =  m_OrthographicSize * 0.5f;

            m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
        }
    }
}