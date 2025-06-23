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

    struct SimpleAnimationComponent : public ComponentBase
    {
    
    public:

        SimpleAnimationComponent() = default;
        ~SimpleAnimationComponent() = default;

        SimpleAnimationComponent(const SimpleAnimationComponent& other);
        SimpleAnimationComponent(SimpleAnimationComponent&& other) noexcept;
        SimpleAnimationComponent& operator=(const SimpleAnimationComponent& other);
        SimpleAnimationComponent& operator=(SimpleAnimationComponent&& other) noexcept;

        ComponentType GetType() override { return ComponentType::SimpleAnimationComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };

    struct AnimatorComponent : public ComponentBase
    {
    
    public:

        AnimatorComponent() = default;
        ~AnimatorComponent() = default;

        AnimatorComponent(const AnimatorComponent& other);
        AnimatorComponent(AnimatorComponent&& other) noexcept;
        AnimatorComponent& operator=(const AnimatorComponent& other);
        AnimatorComponent& operator=(AnimatorComponent&& other) noexcept;

        ComponentType GetType() override { return ComponentType::AnimatorComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
}