#pragma once

// Core Headers
#include "Component Base.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{
    using ShadowType = uint32_t;
    enum : ShadowType
    {
        ShadowType_None = 0,
        ShadowType_Hard = 1,
        ShadowType_Soft = 2
    };
    
    struct SphereLightComponent : public ComponentBase
    {
    
    public:

        SphereLightComponent() = default;
        ~SphereLightComponent() = default;

        SphereLightComponent(const SphereLightComponent& other);
        SphereLightComponent(SphereLightComponent&& other) noexcept;
        SphereLightComponent& operator=(const SphereLightComponent& other);
        SphereLightComponent& operator=(SphereLightComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::SphereLightComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        // Getters
        const glm::vec4& GetColour() const { return m_Colour; }
        float GetRadius() const { return m_Radius; }
        float GetIntensity() const { return m_Intensity; }
        ShadowType GetShadowType() const { return m_ShadowType; }
        bool GetActive() const { return m_Active; }

        // Setters
        void SetColour(const glm::vec4& colour) { m_Colour = colour; }
        void SetRadius(float radius) { m_Radius = radius; }
        void SetIntensity(float intensity) { m_Intensity = intensity; }
        void SetShadowType(ShadowType shadow_type) { m_ShadowType = shadow_type; }
        void SetActive(bool active) { m_Active = active; }

    private:
        
        glm::vec4 m_Colour = glm::vec4(1.0f);
        float m_Radius = 10.0f;
        float m_Intensity = 1.0f;
        ShadowType m_ShadowType = ShadowType_None;
        bool m_Active = true;

    };

    struct ConeLightComponent : public ComponentBase
    {
    
    public:

        ConeLightComponent() = default;
        ~ConeLightComponent() = default;

        ConeLightComponent(const ConeLightComponent& other);
        ConeLightComponent(ConeLightComponent&& other) noexcept;
        ConeLightComponent& operator=(const ConeLightComponent& other);
        ConeLightComponent& operator=(ConeLightComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::ConeLightComponent; }
        
        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        // Getters
        const glm::vec4& GetColour() const { return m_Colour; }
        float GetAngle() const { return m_Angle; }
        float GetRange() const { return m_Range; }
        float GetIntensity() const { return m_Intensity; }
        ShadowType GetShadowType() const { return m_ShadowType; }
        bool GetActive() const { return m_Active; }

        // Setters
        void SetColour(const glm::vec4& colour) { m_Colour = colour; }
        void SetAngle(float angle) { m_Angle = angle; }
        void SetRange(float range) { m_Range = range; }
        void SetIntensity(float intensity) { m_Intensity = intensity; }
        void SetShadowType(ShadowType shadow_type) { m_ShadowType = shadow_type; }
        void SetActive(bool active) { m_Active = active; }

    private:

        glm::vec4 m_Colour = glm::vec4(1.0f);
        float m_Angle = 90.0f;
        float m_Range = 20.0f;
        float m_Intensity = 1.0f;
        ShadowType m_ShadowType = ShadowType_None;
        bool m_Active = true;     

    };

    struct DirectionalLightComponent : public ComponentBase
    {
    
    public:

        DirectionalLightComponent() = default;
        ~DirectionalLightComponent() = default;

        DirectionalLightComponent(const DirectionalLightComponent& other);
        DirectionalLightComponent(DirectionalLightComponent&& other) noexcept;
        DirectionalLightComponent& operator=(const DirectionalLightComponent& other);
        DirectionalLightComponent& operator=(DirectionalLightComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::DirectionalLightComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        // Getters
        const glm::vec4& GetColour() const { return m_Colour; }
        float GetIntensity() const { return m_Intensity; }
        ShadowType GetShadowType() const { return m_ShadowType; }
        bool GetActive() const { return m_Active; }

        // Setters
        void SetColour(const glm::vec4& colour) { m_Colour = colour; }
        void SetIntensity(float intensity) { m_Intensity = intensity; }
        void SetShadowType(ShadowType shadow_type) { m_ShadowType = shadow_type; }
        void SetActive(bool active) { m_Active = active; }

    private:
        
        glm::vec4 m_Colour = glm::vec4(1.0f);
        float m_Intensity = 1.0f;
        ShadowType m_ShadowType = ShadowType_None;
        bool m_Active = true;

    };

}