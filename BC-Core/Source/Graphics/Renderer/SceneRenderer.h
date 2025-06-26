#pragma once

// Core Headers
#include "CameraContext.h"

#include "GeometryEnvironment.h"
#include "LightEnvironment.h"
#include "ShadowEnvironment.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{

    class SceneRenderer
    {

    private:

        SceneRenderer() = delete;
        ~SceneRenderer() = delete;
        
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer(SceneRenderer&&) = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;
        SceneRenderer& operator=(SceneRenderer&&) = delete;

        struct SceneSnapshot
        {
            // CommmandBuffer primary_command_buffer;

            // Key == Camera GUID
            // Value == Camera Environment
            // Edge Case: if Key == PlaceHolderGUID::PLACEHOLDER_0_GUID == EditorCamera
            std::unordered_map<GUID, CameraEnvironment> camera_environments;

            LightEnvironment light_environment;
            ShadowEnvironment shadow_environment;
        };

        struct SceneRenderData
        {
            std::vector<SceneSnapshot> frames_in_flight;

            // Global 
            glm::uvec2 screen_space_size = { 1, 1};

            // CubeMapArray -> MAX_SPHERE_SHADOW_MAPS
            AssetHandle sphere_shadow_map_array; 
            AssetHandle cone_shadow_map_array;
            AssetHandle directional_shadow_map_array;
        };

        static SceneRenderData* s_Data;

    public:

        static void Init();
        static void Shutdown();

        static void SnapshotScene(uint32_t frame_index, const std::vector<CameraContext>& camera_overrides);

        static void RecordCommandBuffers    (uint32_t frame_index);
        static void RecordShadowPass        (CameraEnvironment& camera_env);
        static void RecordDepthPrepass      (CameraEnvironment& camera_env);
        static void RecordComputeCulling    (CameraEnvironment& camera_env);
        static void RecordColourPass        (CameraEnvironment& camera_env);

        static void ResizeScreenSpace(uint32_t width, uint32_t height);
        static const glm::uvec2& GetScreenSpace();


        static void SubmitCommandBuffers(uint32_t frame_index);
    };

}