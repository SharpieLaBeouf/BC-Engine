#include "BC_PCH.h"
#include "EditorCamera.h"

namespace BC
{
    EditorCamera::EditorCamera(uint32_t width, uint32_t height) :
        m_ViewportSize({width, height})
    {
        RenderTargetSpecification specification = {};
        specification.width = width;
        specification.height = height;

        specification.is_swapchain_target = false;
        specification.samples = 4;

        //specification.attachments.push_back({RenderTargetAttachmentFormat::Colour});
        //specification.attachments.push_back({RenderTargetAttachmentFormat::Depth});

        m_RenderTarget = RenderTarget::CreateRenderTarget(specification);

        RecalculateProjection();
        RecalculateView();
    }

    void EditorCamera::OnUpdate()
    {
		static glm::vec2 m_InitialMousePosition = Input::GetMousePosition();

		const glm::vec2& mouse = Input::GetMousePosition();
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if(float scroll_delta = Input::GetScrollY() * 0.1f; scroll_delta != 0.0f)
        {
			MouseZoom(scroll_delta);
        }

		if (Input::GetKey(Key::LeftAlt)) 
        {
			if (Input::GetMouseButton(Button::Mouse_Button_Middle))
            {
				MousePan(delta);
            }
			else if (Input::GetMouseButton(Button::Mouse_Button_Left))
            {
				MouseRotate(delta);
            }
			else if (Input::GetMouseButton(Button::Mouse_Button_Right))
            {
				MouseZoom(delta.y);
            }
		}
		else {

			if (Input::GetMouseButton(Button::Mouse_Button_Middle))
            {
				MousePan(delta);
            }
			else if (Input::GetMouseButton(Button::Mouse_Button_Right))
            {
				MouseRotate(delta);
            }
		}

		RecalculateView();
    }

    void EditorCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        RenderTargetSpecification specification = {};
        specification.width = width;
        specification.height = height;

        m_RenderTarget = RenderTarget::CreateRenderTarget(specification);

        RecalculateProjection();
        RecalculateView();
    }

    const glm::vec3& EditorCamera::GetPosition() const
    {
        return m_Position;
    }

    glm::quat EditorCamera::GetOrientation() const
    {
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    const Frustum& EditorCamera::GetFrustum() const
    {
        return m_Frustum;
    }

    glm::vec3 EditorCamera::GetUpDirection() const
    {
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetRightDirection() const
    {
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetForwardDirection() const
    {
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    void EditorCamera::SetPerspective(float fov, float near_clip, float far_clip)
    {

    }

    void EditorCamera::SetOrthographic(float size, float near_clip, float far_clip)
    {

    }

    void EditorCamera::RecalculateView()
    {
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    void EditorCamera::RecalculateProjection()
    {
        if (m_Type == CameraProjType_Perspective)
        {
            m_AspectRatio = m_ViewportSize.x / m_ViewportSize.y;
            m_ProjectionMatrix = glm::perspective(glm::radians(m_PerspectiveFOV), m_AspectRatio, m_Near, m_Far);
        }
        else
        {
            float orthoLeft     = -m_OrthographicSize * m_AspectRatio * 0.5f;
            float orthoRight    =  m_OrthographicSize * m_AspectRatio * 0.5f;
            float orthoBottom   = -m_OrthographicSize * 0.5f;
            float orthoTop      =  m_OrthographicSize * 0.5f;

            m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_Near, m_Far);
        }
    }

    void EditorCamera::MousePan(const glm::vec2 &delta)
    {
		auto speed = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * speed.x * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * speed.y * m_Distance;
    }

    void EditorCamera::MouseRotate(const glm::vec2 &delta)
    {
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta)
    {
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
    }

    glm::vec3 EditorCamera::CalculatePosition() const
    {
		return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::vec2 EditorCamera::PanSpeed() const
    {
        glm::vec2 result = 
        { 
            std::min(m_ViewportSize.x / 1000.0f, 5.0f), 
            std::min(m_ViewportSize.y / 1000.0f, 5.0f) 
        };

		result.x = 0.0366f * (result.x * result.x) - 0.1778f * result.x + 0.3021f;
		result.y = 0.0366f * (result.y * result.y) - 0.1778f * result.y + 0.3021f;

		return result;
    }

    float EditorCamera::RotationSpeed() const
    {
		return 0.8f;
    }

    float EditorCamera::ZoomSpeed() const
    {
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
    }
}