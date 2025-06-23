#pragma once

// Core Headers
#include "Project/Scene/Entity.h"
#include "Project/Scene/Components/LightComponents.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace BC
{
    namespace Util
    {
        constexpr uint8_t MAX_SPHERE_LIGHT = 1024;
        constexpr uint8_t MAX_CONE_LIGHT = 1024;
        constexpr uint8_t MAX_DIRECTIONAL_LIGHT = 20;

        using LightType = uint32_t;
        enum : LightType
        {
            LightType_Sphere,
            LightType_Cone,
            LightType_Directional
        };

        struct SphereLightData
        {
            LightType light_type;    //  0-3
            float radius;            //  4-7
            uint32_t shadow_type;    //  8-11
            float padding0;          // 12-15
            glm::vec4 colour;        // 16-31 w == intensity
            glm::vec4 position;      // 32-47
            float padding1[4];       // 48-63
        };

        struct ConeLightData
        {
            LightType light_type;    //  0-3
            float angle;             //  4-7
            float range;             //  8-11
            uint32_t shadow_type;    // 12-15
            glm::vec4 colour;        // 16-31 w == intensity
            glm::vec4 position;      // 32-47
            glm::vec4 direction;     // 48-63
        };

        struct DirectionalLightData
        {
            LightType light_type;    //  0-3
            uint32_t shadow_type;    //  4-7
            float padding0[2];       //  8-15
            glm::vec4 colour;        // 16-31 w == intensity
            glm::vec4 position;      // 32-47 
            glm::vec4 direction;     // 48-63
        };

        struct LightData
        {
            union
            {
                SphereLightData sphere;
                ConeLightData cone;
                DirectionalLightData directional;
            };
        };

    }

    struct LightEnvironment
    {
        struct
        {
            uint32_t num_sphere_lights;
            uint32_t num_cone_lights;
            uint32_t num_directional_lights;
            uint32_t total_light_count; // optional
        } LightCounts;

        std::vector<Util::LightData> lights;

        void AddSphereLight(const SphereLightComponent& pl_component)
        {
            if (!pl_component.GetActive())
                return;

            lights.push_back({});
            Util::LightData& data = lights.back();

            data.sphere.light_type = Util::LightType_Sphere;
            data.sphere.radius = pl_component.GetRadius();
            data.sphere.shadow_type = pl_component.GetShadowType();
            data.sphere.colour = pl_component.GetColour();
            data.sphere.colour.w = pl_component.GetIntensity();
            data.sphere.position = glm::vec4(pl_component.GetEntity().GetTransform().GetGlobalPosition(), 1.0f);
        }

        void AddConeLight(const ConeLightComponent& cl_component)
        {
            if (!cl_component.GetActive())
                return;

            lights.push_back({});
            Util::LightData& data = lights.back();

            data.cone.light_type = Util::LightType_Cone;
            data.cone.angle = cl_component.GetAngle();
            data.cone.range = cl_component.GetRange();
            data.cone.shadow_type = cl_component.GetShadowType();
            data.cone.colour = cl_component.GetColour();
            data.cone.colour.w = cl_component.GetIntensity();
            data.cone.position = glm::vec4(cl_component.GetEntity().GetTransform().GetGlobalPosition(), 1.0f);
            data.cone.direction = glm::vec4(cl_component.GetEntity().GetTransform().GetGlobalForwardDirection(), 1.0f);
        }

        void AddDirectionalLight(const DirectionalLightComponent& dl_component)
        {
            if (!dl_component.GetActive())
                return;

            lights.push_back({});
            Util::LightData& data = lights.back();
            
            data.directional.light_type = Util::LightType_Directional;
            data.directional.shadow_type = dl_component.GetShadowType();
            data.directional.colour = dl_component.GetColour();
            data.directional.colour.w = dl_component.GetIntensity();
            data.directional.position = glm::vec4(dl_component.GetEntity().GetTransform().GetGlobalPosition(), 1.0f);
            data.directional.direction = glm::vec4(dl_component.GetEntity().GetTransform().GetGlobalForwardDirection(), 1.0f);
        }

        void SortLights()
        {
            std::vector<Util::LightData> sorted;
            sorted.reserve(lights.size());

            LightCounts = {};

            // Sphere Lights
            for (auto& light : lights)
            {
                if (light.sphere.light_type == Util::LightType_Sphere)
                {
                    sorted.emplace_back(std::move(light));
                    LightCounts.num_sphere_lights++;
                }
            }

            // Cone Lights
            for (auto& light : lights)
            {
                if (light.sphere.light_type == Util::LightType_Cone)
                {
                    sorted.emplace_back(std::move(light));
                    LightCounts.num_cone_lights++;
                }
            }

            // Directional Lights
            for (auto& light : lights)
            {
                if (light.sphere.light_type == Util::LightType_Directional)
                {
                    sorted.emplace_back(std::move(light));
                    LightCounts.num_directional_lights++;
                }
            }

            LightCounts.total_light_count = static_cast<uint32_t>(sorted.size());

            lights = std::move(sorted);
        }

    };
}