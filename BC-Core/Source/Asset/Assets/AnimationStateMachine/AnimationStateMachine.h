#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers


namespace YAML
{
    class Emitter;
    class Node;
}

namespace BC::Animation
{

    class StateMachine : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::AnimationStateMachine; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }

        void DeserialiseAsset(const YAML::Node& data)
        {

        }
        static std::shared_ptr<StateMachine> CreateStateMachine(const std::string& serialised_state_machine_data);

    private:
    

    };

}