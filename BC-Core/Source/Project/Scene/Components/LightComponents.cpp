#include "BC_PCH.h"
#include "LightComponents.h"

// TODO: Implement

namespace BC
{
    
#pragma region SphereLightComponent
    
    SphereLightComponent::SphereLightComponent(const SphereLightComponent& other)
    {

    }

    SphereLightComponent::SphereLightComponent(SphereLightComponent&& other) noexcept
    {

    }

    SphereLightComponent& SphereLightComponent::operator=(const SphereLightComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    SphereLightComponent& SphereLightComponent::operator=(SphereLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void SphereLightComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool SphereLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region ConeLightComponent
    
    ConeLightComponent::ConeLightComponent(const ConeLightComponent& other)
    {

    }

    ConeLightComponent::ConeLightComponent(ConeLightComponent&& other) noexcept
    {

    }

    ConeLightComponent& ConeLightComponent::operator=(const ConeLightComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    ConeLightComponent& ConeLightComponent::operator=(ConeLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void ConeLightComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool ConeLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region DirectionalLightComponent
    
    DirectionalLightComponent::DirectionalLightComponent(const DirectionalLightComponent& other)
    {

    }

    DirectionalLightComponent::DirectionalLightComponent(DirectionalLightComponent&& other) noexcept
    {

    }

    DirectionalLightComponent& DirectionalLightComponent::operator=(const DirectionalLightComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    DirectionalLightComponent& DirectionalLightComponent::operator=(DirectionalLightComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void DirectionalLightComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool DirectionalLightComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
