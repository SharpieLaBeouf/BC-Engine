#include "BC_PCH.h"
#include "AudioComponents.h"

#include <yaml-cpp/yaml.h>

// TODO: Implement

namespace BC
{
    
#pragma region AudioListenerComponent
    
    AudioListenerComponent::AudioListenerComponent(const AudioListenerComponent& other)
    {

    }

    AudioListenerComponent::AudioListenerComponent(AudioListenerComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

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

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }

    void AudioListenerComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{

        }
		out << YAML::EndMap;
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
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

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
            
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }

    void AudioEmitterComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool AudioEmitterComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
