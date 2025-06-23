#pragma once

// Core Headers
#include "LightEnvironment.h"
#include "GeometryEnvironment.h"

#include "Asset/Asset.h"
#include "Project/Scene/Components/LightComponents.h"

// C++ Standard Library Headers
#include <vector>
#include <unordered_map>

// External Vendor Headers
#include <glm/glm.hpp>

namespace BC
{
    namespace Util
    {
        constexpr uint8_t MAX_SPHERE_SHADOW_MAPS    = 32;
        constexpr uint8_t MAX_CONE_SHADOW_MAPS      = 32;
        constexpr uint8_t MAX_DIRECTIONAL_SHADOW_MAPS      = 32;

        struct SphereShadow
        {
            uint32_t light_index; // Index into LightEnvironment
            glm::vec3 position;

            // Geometry that casts shadows for this light
            GeometryEnvironment geometry;

            // Face index 0–5 will be iterated over when rendering
            // Render target is pulled from global scene-wide cube shadow map
        };

        struct ConeShadow
        {
            uint32_t light_index;
            glm::vec3 position;
            glm::vec3 direction;
            glm::mat4 view_proj;

            GeometryEnvironment geometry;

            // Render target is a slice in global 2D array shadow map
        };

        struct DirectionalCascade
        {
            glm::mat4 view_proj;
            float split_depth;
            uint32_t cascade_index;
            AssetHandle shadow_map_handle; // Layered 2D array, local to CameraEnvironment
        };

        struct DirectionalShadow
        {
            uint32_t light_index;
            glm::vec3 direction;
            glm::vec3 position;

            GeometryEnvironment geometry;

            std::vector<DirectionalCascade> cascades;
        };
    }

    struct ShadowEnvironment
    {
        std::vector<Util::SphereShadow> sphere_shadows;
        std::vector<Util::ConeShadow> cone_shadows;
        std::vector<Util::DirectionalShadow> directional_shadows;
    };
}
