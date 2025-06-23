#include "BC_PCH.h"
#include "SceneRenderer.h"

#include "Asset/Assets/RenderTarget.h"
#include "Asset/AssetManagerAPI.h"

namespace BC
{

    namespace Util
    {
        bool IsRectContained(const glm::vec4& outer, const glm::vec4& inner)
        {
            return
                inner.x >= outer.x &&
                inner.y >= outer.y &&
                inner.x + inner.z <= outer.x + outer.z &&
                inner.y + inner.w <= outer.y + outer.w;
        }

        static void GatherCamerasHelper(std::vector<CameraContext>& cam_ctxs)
        {
            auto camera_view = Application::GetProject()->GetSceneManager()->GetAllEntitiesWithComponent<CameraComponent>();
            for (const auto& entity : camera_view)
            {
                CameraContext ctx = {};
                CameraComponent& cam_component = entity.GetComponent<CameraComponent>();

                // If Render Target Handle Not Valid
                if (!AssetManager::IsAssetHandleValid(cam_component.GetRenderTarget()) || (!cam_component.GetShouldDisplay() && !cam_component.GetForceRender()))
                    continue;

                // Validate camera render target
                auto render_target_asset = AssetManager::GetAsset<RenderTarget>(cam_component.GetRenderTarget());
                if (!render_target_asset || !render_target_asset->IsValid())
                    continue;
                
                auto& transform = entity.GetComponent<TransformComponent>();
                auto camera_ref = cam_component.GetCamera();

                cam_ctxs.push_back({});
                auto& preview_ctx = cam_ctxs.back();

                preview_ctx.transform_matrix = transform.GetGlobalMatrix();
                preview_ctx.view_matrix = glm::inverse(preview_ctx.transform_matrix);
                preview_ctx.projection_matrix = camera_ref->GetProjMatrix();

                preview_ctx.camera_frustum = Frustum(preview_ctx.projection_matrix * preview_ctx.view_matrix);

                preview_ctx.owning_camera_entity = entity.GetGUID();
                preview_ctx.is_editor_camera = false;

                preview_ctx.render_target = cam_component.GetRenderTarget();
                preview_ctx.composite_depth = cam_component.GetDepth();
                preview_ctx.composite_viewport = cam_component.GetViewport();

                preview_ctx.should_display = cam_component.GetShouldDisplay();
                preview_ctx.force_render = cam_component.GetForceRender();

                preview_ctx.aspect_ratio = camera_ref->GetAspectRatio();

                preview_ctx.is_orthographic = camera_ref->GetType() == SceneCamera::CameraProjType_Orthographic;
                if (preview_ctx.is_orthographic)
                {
                    preview_ctx.near_clip = camera_ref->GetOrthographicNearClip();
                    preview_ctx.far_clip = camera_ref->GetOrthographicFarClip();

                    preview_ctx.orthographic_size = camera_ref->GetOrthographicSize();
                }
                else
                {
                    preview_ctx.near_clip = camera_ref->GetPerspectiveNearClip();
                    preview_ctx.far_clip = camera_ref->GetPerspectiveFarClip();

                    preview_ctx.fov = camera_ref->GetPerspectiveFOV();
                }
            }

            std::sort(cam_ctxs.begin(), cam_ctxs.end(), [](const CameraContext& a, const CameraContext& b)
            {
                return a.composite_depth > b.composite_depth;
            });

            std::vector<CameraContext> final_cameras;
            final_cameras.resize(cam_ctxs.size());
            for (auto it = cam_ctxs.begin(); it != cam_ctxs.end(); )
            {
                if (!it->should_display && !it->force_render)
                {
                    it = cam_ctxs.erase(it);
                    continue;
                }

                if (it->force_render)
                {
                    ++it;
                    continue;
                }
                    
                bool covered = false;
                for (const auto& higher : final_cameras)
                {
                    if (IsRectContained(higher.composite_viewport, it->composite_viewport))
                    {
                        covered = true;
                        break;
                    }
                }

                if (covered)
                {
                    it = cam_ctxs.erase(it);
                }
                else
                {
                    final_cameras.push_back(*it);
                    ++it;
                }
            }
            cam_ctxs = std::move(final_cameras);
        }

    }

    SceneRenderer::SceneRenderData* SceneRenderer::s_Data = nullptr;

	void SceneRenderer::Init()
	{
        if (!s_Data)
            s_Data = new SceneRenderer::SceneRenderData();
        
        s_Data->frames_in_flight.clear();
        s_Data->frames_in_flight.resize(Application::GetVulkanCore()->GetSwapchain().GetImageCount());
	}

	void SceneRenderer::Shutdown()
	{
        s_Data->frames_in_flight.clear();
        if (s_Data)
        {
            delete s_Data;
            s_Data = nullptr;
        }
	}

    void SceneRenderer::SnapshotScene(uint32_t frame_index, const std::vector<CameraContext>& camera_overrides)
    {
        auto scene_manager  = Application::GetProject()->GetSceneManager();

        auto& frame         = s_Data->frames_in_flight[frame_index];
        auto& cam_envs      = frame.camera_environments;
        auto& light_env     = frame.light_environment;
        auto& shadow_env    = frame.shadow_environment;

        // 1. Camera Environments
        std::vector<CameraContext> cam_ctxs = camera_overrides;
        if (cam_ctxs.empty())
        {
            Util::GatherCamerasHelper(cam_ctxs);

            // Check again to see if we should early return
            if (cam_ctxs.empty())
            {
                return;
            }
        }

        JobCounter gather_lights = {};
        Application::GetJobSystem()->SubmitJob
        (
            "SnapshotScene - Gather Visible Lights",
            [&]()
            {
                auto sphere_light_view = scene_manager->GetAllEntitiesWithComponent<SphereLightComponent>();
                for (const auto& entity : sphere_light_view)
                {
                    auto& component = entity.GetComponent<SphereLightComponent>();
                    if (!component.GetActive())
                        continue;
                    
                    for (const auto& context : cam_ctxs)
                    {
                        if (context.camera_frustum.Contains(Bounds_Sphere{TransformComponent::GetPositionFromMatrix(context.transform_matrix), component.GetRadius()}) != FrustumContainResult::DoesNotContain)
                        {
                            light_env.AddSphereLight(component);
                            break;
                        }
                    }
                }

                auto cone_light_view = scene_manager->GetAllEntitiesWithComponent<ConeLightComponent>();
                for (const auto& entity : cone_light_view)
                {
                    auto& component = entity.GetComponent<ConeLightComponent>();
                    if (!component.GetActive())
                        continue;
                    
                    for (const auto& context : cam_ctxs)
                    {
                        float half_angle = glm::radians(component.GetAngle() * 0.5f);
                        float cos_penumbra = cos(half_angle);

                        Bounds_Sphere sphere;
                        if (half_angle > glm::pi<float>() / 4.0f) 
                        {
                            sphere.BoundsCentre = TransformComponent::GetPositionFromMatrix(context.transform_matrix) + cos_penumbra * component.GetRange() * TransformComponent::GetForwardDirectionFromMatrix(context.transform_matrix);
                            sphere.BoundsRadius = sin(half_angle) * component.GetRange();
                        }
                        else
                        {
                            sphere.BoundsCentre = TransformComponent::GetPositionFromMatrix(context.transform_matrix) + component.GetRange() / (2.0f * cos_penumbra) * TransformComponent::GetForwardDirectionFromMatrix(context.transform_matrix);
                            sphere.BoundsRadius = component.GetRange() / (2.0f * cos_penumbra);
                        }

                        if (context.camera_frustum.Contains(sphere) != FrustumContainResult::DoesNotContain)
                        {
                            light_env.AddConeLight(component);
                            break;
                        }
                    }
                }

                int directional_lights_added = 0;
                auto directional_light_view = scene_manager->GetAllEntitiesWithComponent<DirectionalLightComponent>();
                for (const auto& entity : directional_light_view) 
                {
                    if (directional_lights_added >= Util::MAX_DIRECTIONAL_LIGHT)
                        break;

                    auto& component = entity.GetComponent<DirectionalLightComponent>();
                    if (!component.GetActive())
                        continue;

                    light_env.AddDirectionalLight(component);
                    directional_lights_added++;
                }
            },
            &gather_lights,
            JobPriority::Low,
            false
        );

        JobCounter gather_shadow_casters = {};
        Application::GetJobSystem()->SubmitJob
        (
            "SnapshotScene - Gather Visible Shadows",
            [&]()
            {
                auto light_view = scene_manager->GetAllEntitiesWithComponent<SphereLightComponent, ConeLightComponent, DirectionalLightComponent>();
                for (const auto& entity : light_view)
                {
                }
            },
            &gather_shadow_casters,
            JobPriority::Low,
            false
        );

        for (const auto& context : cam_ctxs)
        {

        }
    }

    void SceneRenderer::RecordCommandBuffers(uint32_t frame_index)
    {
    }

    void SceneRenderer::SubmitCommandBuffers(uint32_t frame_index)
    {
    }
}
