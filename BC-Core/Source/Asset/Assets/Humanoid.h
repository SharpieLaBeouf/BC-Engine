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

namespace BC
{

    class Humanoid : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Humanoid; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }

        void DeserialiseAsset(const YAML::Node& data)
        {

        }

        static std::shared_ptr<Humanoid> CreateHumanoid(const std::string& serialised_humanoid_data);

    private:
    

    };

    class HumanoidMask : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::HumanoidMask; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }
        
        void DeserialiseAsset(const YAML::Node& data)
        {

        }

        static std::shared_ptr<HumanoidMask> CreateHumanoidMask(const std::string& serialised_humanoid_mask_data);

    private:

        

    };

}