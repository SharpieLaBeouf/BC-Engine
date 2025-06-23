#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{
    using MaterialRenderType = uint8_t;
	enum : MaterialRenderType 
    {
        MaterialRenderType_Opaque,
        MaterialRenderType_Mixed,
        MaterialRenderType_Transparent
	};

    class Material : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Material_Standard; }

        MaterialRenderType GetMaterialRenderType() const { return m_RenderType; }

    private:

        MaterialRenderType m_RenderType;

    };

}