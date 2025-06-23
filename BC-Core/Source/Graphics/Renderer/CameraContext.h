#pragma once

#include <glm/glm.hpp>

// Core Headers

#include "Core/GUID.h"

#include "GeometryEnvironment.h"
#include "LightEnvironment.h"
#include "ShadowEnvironment.h"

#include "Project/Scene/Bounds/Frustum.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{
    struct CameraContext
    {
        glm::mat4 transform_matrix      = glm::mat4(1.0f);
        glm::mat4 view_matrix           = glm::mat4(1.0f);
        glm::mat4 projection_matrix     = glm::mat4(1.0f);

        Frustum camera_frustum          = {};

        // Quick reference if we need anything else
        GUID owning_camera_entity       = NULL_GUID;

        // Can be used to disable post-processing, debug UI etc.
        bool is_editor_camera           = false;

        // Camera Render Target constructed and destroyed by CameraComponent (in BC-Core) and EditorCamera (in BC-Editor)
        AssetHandle render_target       = NULL_GUID;

        // The depth of the camera to be displayed in the final composite 0 == back most, > 1 front most
        uint8_t composite_depth         = 0;

        // Where on the final display will the image be drawn, default will take up full screen, but glm::vec4(0.0f, 0.0f, 0.5f, 1.0f) would take up left most half of screen
        glm::vec4 composite_viewport    = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // x = xpos, y = ypos, z = width normalised, w = height normalised
        
        bool should_display              = true;
        bool force_render               = false;

        float aspect_ratio              = 16.0f / 9.0f; // Default 16:9 aspect

        // If Perspective
        float fov                       = 60.0f;

        // If Ortho
        bool is_orthographic            = false;
        float orthographic_size         = 10.0f;
        
        float near_clip                 = 0.1f;
        float far_clip                  = 1000.0f;

        glm::vec3 GetWorldPosition() const
        {
            return glm::vec3(transform_matrix[3]);
        }

        glm::vec3 GetForwardRH() const
        {
            return glm::normalize(glm::vec3(transform_matrix[2]) * -1.0f);
        }
    };

    struct CameraEnvironment
    {
        struct
        {
            // CommandBuffer depth_prepass_cmd;
            // CommandBuffer shadow_pass_cmd;
            // CommandBuffer compute_light_cull_cmd;
            // CommandBuffer color_pass_cmd;
            // CommandBuffer postprocess_cmd;
        } secondary_command_buffers;

        // SSBOBuffer sphere_light_indices;
        // SSBOBuffer cone_light_indices;

        GeometryEnvironment geometry_environment;
        CameraContext camera_ctx;
    };

}