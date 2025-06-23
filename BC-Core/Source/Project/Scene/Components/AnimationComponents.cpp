#include "BC_PCH.h"
#include "AnimationComponents.h"

namespace BC
{

// TODO: Implement
    
#pragma region SimpleAnimationComponent
    
    SimpleAnimationComponent::SimpleAnimationComponent(const SimpleAnimationComponent& other)
    {

    }

    SimpleAnimationComponent::SimpleAnimationComponent(SimpleAnimationComponent&& other) noexcept
    {

    }

    SimpleAnimationComponent& SimpleAnimationComponent::operator=(const SimpleAnimationComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    SimpleAnimationComponent& SimpleAnimationComponent::operator=(SimpleAnimationComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void SimpleAnimationComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool SimpleAnimationComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region AnimatorComponent
    
    AnimatorComponent::AnimatorComponent(const AnimatorComponent& other)
    {

    }

    AnimatorComponent::AnimatorComponent(AnimatorComponent&& other) noexcept
    {

    }

    AnimatorComponent& AnimatorComponent::operator=(const AnimatorComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    AnimatorComponent& AnimatorComponent::operator=(AnimatorComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    void AnimatorComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool AnimatorComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}