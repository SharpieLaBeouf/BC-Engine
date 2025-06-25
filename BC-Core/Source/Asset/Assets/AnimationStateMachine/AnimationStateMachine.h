#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC::Animation
{

    class StateMachine : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::AnimationStateMachine; }

    private:
    

    };

}