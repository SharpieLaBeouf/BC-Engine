#pragma once

// Core Headers
#include "Component Base.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{

    struct AudioListenerComponent : public ComponentBase
    {
    
    public:

        AudioListenerComponent() = default;
        ~AudioListenerComponent() = default;

        AudioListenerComponent(const AudioListenerComponent& other);
        AudioListenerComponent(AudioListenerComponent&& other) noexcept;
        AudioListenerComponent& operator=(const AudioListenerComponent& other);
        AudioListenerComponent& operator=(AudioListenerComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::AudioListenerComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };

    struct AudioEmitterComponent : public ComponentBase
    {
    
    public:

        AudioEmitterComponent() = default;
        ~AudioEmitterComponent() = default;

        AudioEmitterComponent(const AudioEmitterComponent& other);
        AudioEmitterComponent(AudioEmitterComponent&& other) noexcept;
        AudioEmitterComponent& operator=(const AudioEmitterComponent& other);
        AudioEmitterComponent& operator=(AudioEmitterComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::AudioEmitterComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
}