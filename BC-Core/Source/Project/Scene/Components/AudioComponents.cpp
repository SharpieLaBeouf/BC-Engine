#include "BC_PCH.h"
#include "AudioComponents.h"

// TODO: Implement

namespace BC
{
    
#pragma region AudioListenerComponent
    
    AudioListenerComponent::AudioListenerComponent(const AudioListenerComponent& other)
    {

    }

    AudioListenerComponent::AudioListenerComponent(AudioListenerComponent&& other) noexcept
    {

    }

    AudioListenerComponent& AudioListenerComponent::operator=(const AudioListenerComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    AudioListenerComponent& AudioListenerComponent::operator=(AudioListenerComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void AudioListenerComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool AudioListenerComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region AudioEmitterComponent
    
    AudioEmitterComponent::AudioEmitterComponent(const AudioEmitterComponent& other)
    {

    }

    AudioEmitterComponent::AudioEmitterComponent(AudioEmitterComponent&& other) noexcept
    {

    }

    AudioEmitterComponent& AudioEmitterComponent::operator=(const AudioEmitterComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    AudioEmitterComponent& AudioEmitterComponent::operator=(AudioEmitterComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void AudioEmitterComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool AudioEmitterComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
