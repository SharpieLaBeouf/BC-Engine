#include "BC_PCH.h"
#include "LightComponents.h"

#include <yaml-cpp/yaml.h>

// TODO: Implement

namespace BC
{
    
#pragma region SphereLightComponent
    
    SphereLightComponent::SphereLightComponent(const SphereLightComponent& other)
    {
        m_Colour = other.m_Colour;
        m_Radius = other.m_Radius;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;
    }

    SphereLightComponent::SphereLightComponent(SphereLightComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Colour = std::move(other.m_Colour);
        m_Radius = other.m_Radius;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Radius = 10.0f;
        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;
    }

    SphereLightComponent& SphereLightComponent::operator=(const SphereLightComponent& other)
    {
        if (this == &other)
            return *this;

        m_Colour = other.m_Colour;
        m_Radius = other.m_Radius;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        return *this;
    }

    SphereLightComponent& SphereLightComponent::operator=(SphereLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;
        
        m_Colour = std::move(other.m_Colour);
        m_Radius = other.m_Radius;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Radius = 10.0f;
        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;

        return *this;
    }

    void SphereLightComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool SphereLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region ConeLightComponent
    
    ConeLightComponent::ConeLightComponent(const ConeLightComponent& other)
    {
        m_Colour = other.m_Colour;
        m_Angle = other.m_Angle;
        m_Range = other.m_Range;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;
    }

    ConeLightComponent::ConeLightComponent(ConeLightComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;
        
        m_Colour = std::move(other.m_Colour);
        m_Angle = other.m_Angle;
        m_Range = other.m_Range;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Angle = 90.0f;
        other.m_Range = 20.0f;
        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;

    }

    ConeLightComponent& ConeLightComponent::operator=(const ConeLightComponent& other)
    {
        if (this == &other)
            return *this;

        m_Colour = other.m_Colour;
        m_Angle = other.m_Angle;
        m_Range = other.m_Range;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        return *this;
    }

    ConeLightComponent& ConeLightComponent::operator=(ConeLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;
        
        m_Colour = std::move(other.m_Colour);
        m_Angle = other.m_Angle;
        m_Range = other.m_Range;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Angle = 90.0f;
        other.m_Range = 20.0f;
        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;

        return *this;
    }

    void ConeLightComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool ConeLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region DirectionalLightComponent
    
    DirectionalLightComponent::DirectionalLightComponent(const DirectionalLightComponent& other)
    {
        m_Colour = other.m_Colour;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;
    }

    DirectionalLightComponent::DirectionalLightComponent(DirectionalLightComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Colour = std::move(other.m_Colour);
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;
    }

    DirectionalLightComponent& DirectionalLightComponent::operator=(const DirectionalLightComponent& other)
    {
        if (this == &other)
            return *this;
            
        m_Colour = other.m_Colour;
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        return *this;
    }

    DirectionalLightComponent& DirectionalLightComponent::operator=(DirectionalLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;
        
        m_Colour = std::move(other.m_Colour);
        m_Intensity = other.m_Intensity;
        m_ShadowType = other.m_ShadowType;
        m_Active = other.m_Active;

        other.m_Intensity = 1.0f;
        other.m_ShadowType = ShadowType_None;
        other.m_Active = true;

        return *this;
    }

    void DirectionalLightComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool DirectionalLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
