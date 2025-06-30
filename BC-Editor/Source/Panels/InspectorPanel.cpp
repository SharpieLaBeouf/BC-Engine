#include "BC_PCH.h"
#include "InspectorPanel.h"

namespace BC
{

    static std::vector<const char*> s_ClearTypeStrings = 
    {
        "Colour",
        "Skybox"
    };

    static std::vector<const char*> s_ShadowTypeStrings = 
    {
        "None",
        "Hard Shadows",
        "Soft Shadows"
    };

    static std::vector<const char*> s_AnimationCullingModeStrings = 
    {
        "Always",
        "Update Offscreen",
        "Stop Update Offscreen"
    };

#pragma region Component Draw Functions

    static void DrawTransformComponent(TransformComponent& component)
    {
        // Do Nothing
    }

    static void DrawMetaComponent(MetaComponent& component)
    {
        // Do Nothing
    }

    static void DrawCameraComponent(CameraComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##CameraComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Should Display
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Should Display");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Should this camera display to final scene space render. If not toggle ForceRender to use for offscreen rendering.");
                
                ImGui::TableSetColumnIndex(1);

                auto should_display = component.GetShouldDisplay();
                if (ImGui::Checkbox("##ShouldDisplay", &should_display))
                {
                    component.SetShouldDisplay(should_display);
                }
            }

            // Force Render
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Force Render");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("This will force the camera to be rendered every frame.");
                
                ImGui::TableSetColumnIndex(1);

                auto force_render = component.GetForceRender();
                if (ImGui::Checkbox("##ForceRender", &force_render))
                {
                    component.SetForceRender(force_render);
                }
            }
            
            // Camera Depth
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("CameraDepth");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The depth of the camera in the final screen space render. Lower will be behind other cameras with higher depths.");
                
                ImGui::TableSetColumnIndex(1);

                auto depth = component.GetDepth();
                static uint8_t max_depth = std::numeric_limits<uint8_t>::max();
                if (ImGui::DragScalar("##CameraDepth", ImGuiDataType_U8, &depth, 0.1f, nullptr, &max_depth))
                {
                    component.SetDepth(depth);
                }
            }

            // Clear Type
            int current_clear_type = static_cast<int>(component.GetClearType());
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Clear Type");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("How should the camera clear, with colour or skybox?");
                
                ImGui::TableSetColumnIndex(1);
                
                if (ImGui::Combo("##ClearTypeCombo", &current_clear_type, s_ClearTypeStrings.data(), static_cast<int>(s_ClearTypeStrings.size())))
                {
                    component.SetClearType(static_cast<CameraComponent::CameraClearType>(current_clear_type));
                }
            }

            // Clear Colour
            if (current_clear_type == CameraComponent::CameraClearType_Colour)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Clear Colour");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The colour the screen will be cleared with when starting the render pass.");
                
                ImGui::TableSetColumnIndex(1);

                auto clear_colour = component.GetClearColour();
                if (ImGui::ColorEdit4("##ClearColour", glm::value_ptr(clear_colour)))
                {
                    component.SetClearColour(clear_colour);
                }
            }

            // Skybox Handle
            if (current_clear_type == CameraComponent::CameraClearType_Skybox)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Skybox Material");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The skybox that will be rendered as the scene background/backdrop.");
                
                ImGui::TableSetColumnIndex(1);

                std::string label = "None (Skybox Asset)";
                AssetHandle handle = component.GetSkyboxHandle();
                if (AssetManager::IsAssetHandleValid(handle))
                {
                    auto meta_data = AssetManager::GetMetaData(handle);
                    label = meta_data.name + " (Skybox Asset)";
                }

                char label_buf[256] = {};
                strncpy(label_buf, label.c_str(), sizeof(label_buf) - 1);

                ImGui::InputText("##SkyboxAsset", label_buf, sizeof(label_buf), ImGuiInputTextFlags_ReadOnly);
                
                if (ImGui::BeginDragDropTarget())
                {
                    AssetHandle handle = NULL_GUID;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FILE"))
                    {
                        // Convert the payload data (string) back into a filesystem path
                        std::filesystem::path dropped_path = std::string(static_cast<const char*>(payload->Data), payload->DataSize);

                        if (std::filesystem::exists(dropped_path))
                        {
                            auto meta_data = AssetManager::GetMetaData(dropped_path);
                            handle = meta_data.handle;
                        }
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                    {
                        handle = *static_cast<const AssetHandle*>(payload->Data);
                    }

                    if (handle != NULL_GUID && AssetManager::IsAssetHandleValid(handle))
                    {
                        if (AssetManager::GetMetaData(handle).type == AssetType::Material_Skybox)
                        {
                            component.SetSkyboxHandle(handle);
                        }
                        else
                        {
                            BC_APP_TRACE("InspectorPanel::DrawCameraComponent: Dropped Invalid Asset onto Skybox Handle.");
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }

            // Viewport
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Viewport");
                
                ImGui::TableSetColumnIndex(1);

                bool view_port_modified = false;
                auto view_port = component.GetViewport();
                ImGui::TextUnformatted("Position");
                ImGui::SameLine();
                if (ImGui::DragFloat2("##ViewportPosition", &view_port[0], 0.01f, 0.0f, 0.0f, "%.2f"))
                {
                    view_port_modified = true;
                }
                ImGui::TextUnformatted("Size    "); // Add spacing to align to position
                ImGui::SameLine();
                if (ImGui::DragFloat2("##ViewportSize", &view_port[2], 0.01f, 0.01f, 1.0f, "%.2f")) // clamp between 0.01->1.0f == 1% of screen space smallest size
                {
                    view_port_modified = true;
                }

                if (view_port_modified)
                    component.SetViewport(view_port, SceneRenderer::GetScreenSpace());
            }
            
            ImGui::EndTable();
        }
    
    }

    static void DrawAudioListenerComponent(AudioListenerComponent& component)
    {

    }

    static void DrawAudioEmitterComponent(AudioEmitterComponent& component)
    {

    }

    static void DrawLODMeshComponent(LODMeshComponent& component)
    {

    }

    static void DrawMeshRendererComponent(MeshRendererComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##MeshRendererComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Active
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Active");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Mesh to be Rendered?");
                
                ImGui::TableSetColumnIndex(1);

                auto active = component.GetActive();
                if (ImGui::Checkbox("##MeshRendererActive", &active))
                {
                    component.SetActive(active);
                }
            }

            // Casting Shadows
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Cast Shadows");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Mesh to Cast Shadows?");
                
                ImGui::TableSetColumnIndex(1);

                auto cast_shadows = component.GetCastingShadows();
                if (ImGui::Checkbox("##MeshRendererCastShadows", &cast_shadows))
                {
                    component.SetCastingShadows(cast_shadows);
                }
            }

            // Debug AABB
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Draw Debug Box");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Draw the Debug AABB.");
                
                ImGui::TableSetColumnIndex(1);

                auto draw_debug = component.GetDrawDebug();
                if (ImGui::Checkbox("##MeshRendererDrawDebugAABB", &draw_debug))
                {
                    component.SetDrawDebug(draw_debug);
                }
            }

            // Mesh Handle
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Mesh");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The mesh that will be rendered.");
                
                ImGui::TableSetColumnIndex(1);

                std::string label = "None (Mesh Asset)";
                AssetHandle handle = component.GetMesh();
                if (AssetManager::IsAssetHandleValid(handle))
                {
                    auto meta_data = AssetManager::GetMetaData(handle);
                    label = meta_data.name + " (Mesh Asset)";
                }

                char label_buf[256] = {};
                strncpy(label_buf, label.c_str(), sizeof(label_buf) - 1);

                ImGui::InputText("##MeshRendererMeshAsset", label_buf, sizeof(label_buf), ImGuiInputTextFlags_ReadOnly);
                
                if (ImGui::BeginDragDropTarget())
                {
                    AssetHandle handle = NULL_GUID;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FILE"))
                    {
                        // Convert the payload data (string) back into a filesystem path
                        std::filesystem::path dropped_path = std::string(static_cast<const char*>(payload->Data), payload->DataSize);

                        if (std::filesystem::exists(dropped_path))
                        {
                            auto meta_data = AssetManager::GetMetaData(dropped_path);
                            handle = meta_data.handle;
                        }
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                    {
                        handle = *static_cast<const AssetHandle*>(payload->Data);
                    }

                    if (handle != NULL_GUID && AssetManager::IsAssetHandleValid(handle))
                    {
                        if (AssetManager::GetMetaData(handle).type == AssetType::Mesh)
                        {
                            component.SetMesh(handle);
                        }
                        else
                        {
                            BC_APP_TRACE("InspectorPanel::DrawMeshRendererComponent: Dropped Invalid Asset onto Mesh Handle.");
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::EndTable();
        }
    }

    static void DrawSkinnedMeshRendererComponent(SkinnedMeshRendererComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##SkinnedMeshRendererComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Active
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Active");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Skinned Mesh to be Rendered?");
                
                ImGui::TableSetColumnIndex(1);

                auto active = component.GetActive();
                if (ImGui::Checkbox("##SkinnedMeshRendererActive", &active))
                {
                    component.SetActive(active);
                }
            }

            // Casting Shadows
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Cast Shadows");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Skinned Mesh to Cast Shadows?");
                
                ImGui::TableSetColumnIndex(1);

                auto cast_shadows = component.GetCastingShadows();
                if (ImGui::Checkbox("##SkinnedMeshRendererCastShadows", &cast_shadows))
                {
                    component.SetCastingShadows(cast_shadows);
                }
            }

            // Debug AABB
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Draw Debug Box");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Draw the Debug AABB.");
                
                ImGui::TableSetColumnIndex(1);

                auto draw_debug = component.GetDrawDebug();
                if (ImGui::Checkbox("##SkinnedMeshRendererDrawDebugAABB", &draw_debug))
                {
                    component.SetDrawDebug(draw_debug);
                }
            }

            // Mesh Handle
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Mesh");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The skinned mesh that will be rendered.");
                
                ImGui::TableSetColumnIndex(1);

                std::string label = "None (Mesh Asset)";
                AssetHandle handle = component.GetMesh();
                if (AssetManager::IsAssetHandleValid(handle))
                {
                    auto meta_data = AssetManager::GetMetaData(handle);
                    label = meta_data.name + " (Mesh Asset)";
                }

                char label_buf[256] = {};
                strncpy(label_buf, label.c_str(), sizeof(label_buf) - 1);

                ImGui::InputText("##SkinnedMeshRendererMeshAsset", label_buf, sizeof(label_buf), ImGuiInputTextFlags_ReadOnly);
                
                if (ImGui::BeginDragDropTarget())
                {
                    AssetHandle handle = NULL_GUID;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FILE"))
                    {
                        // Convert the payload data (string) back into a filesystem path
                        std::filesystem::path dropped_path = std::string(static_cast<const char*>(payload->Data), payload->DataSize);

                        if (std::filesystem::exists(dropped_path))
                        {
                            auto meta_data = AssetManager::GetMetaData(dropped_path);
                            handle = meta_data.handle;
                        }
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                    {
                        handle = *static_cast<const AssetHandle*>(payload->Data);
                    }

                    if (handle != NULL_GUID && AssetManager::IsAssetHandleValid(handle))
                    {
                        if (AssetManager::GetMetaData(handle).type == AssetType::Mesh)
                        {
                            component.SetMesh(handle);
                        }
                        else
                        {
                            BC_APP_TRACE("InspectorPanel::DrawSkinnedMeshRendererComponent: Dropped Invalid Asset onto Mesh Handle.");
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::EndTable();
        }
    }

    static void DrawSimpleAnimationComponent(SimpleAnimationComponent& component)
    {

    }

    static void DrawAnimatorComponent(AnimatorComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##AnimatorComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Animator Handle
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Animator");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The Animator State Machine Asset.");
                
                ImGui::TableSetColumnIndex(1);

                std::string label = "None (Animator Asset)";
                AssetHandle handle = component.GetStateMachineHandle();
                if (AssetManager::IsAssetHandleValid(handle))
                {
                    auto meta_data = AssetManager::GetMetaData(handle);
                    label = meta_data.name + " (Animator Asset)";
                }

                char label_buf[256] = {};
                strncpy(label_buf, label.c_str(), sizeof(label_buf) - 1);

                ImGui::InputText("##AnimatorAsset", label_buf, sizeof(label_buf), ImGuiInputTextFlags_ReadOnly);
                
                if (ImGui::BeginDragDropTarget())
                {
                    AssetHandle handle = NULL_GUID;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_ITEM_FILE"))
                    {
                        // Convert the payload data (string) back into a filesystem path
                        std::filesystem::path dropped_path = std::string(static_cast<const char*>(payload->Data), payload->DataSize);

                        if (std::filesystem::exists(dropped_path))
                        {
                            auto meta_data = AssetManager::GetMetaData(dropped_path);
                            handle = meta_data.handle;
                        }
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                    {
                        handle = *static_cast<const AssetHandle*>(payload->Data);
                    }

                    if (handle != NULL_GUID && AssetManager::IsAssetHandleValid(handle))
                    {
                        if (AssetManager::GetMetaData(handle).type == AssetType::Mesh)
                        {
                            component.SetStateMachineHandle(handle);
                        }
                        else
                        {
                            BC_APP_TRACE("InspectorPanel::DrawAnimatorComponent: Dropped Invalid Asset onto Animator Handle.");
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }

            // Animator Cull Mode
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Culling Mode");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Always = Always will animate (less performant)\nUpdate Offscreen = Will update animation state when offscreen\nStop Update Offscreen = Will stop animation state updates entirely when offscreen");
                
                ImGui::TableSetColumnIndex(1);
                
                int current_mode = static_cast<int>(component.GetCullingMode());
                if (ImGui::Combo("##AnimatorCullingMode", &current_mode, s_AnimationCullingModeStrings.data(), static_cast<int>(s_AnimationCullingModeStrings.size())))
                {
                    component.SetCullingMode(static_cast<AnimatorComponent::AnimationCullingMode>(current_mode));
                }
            }

            ImGui::EndTable();
        }
    }

    static void DrawSphereLightComponent(SphereLightComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##SphereLightComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Active
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Active");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Light On?");
                
                ImGui::TableSetColumnIndex(1);

                auto active = component.GetActive();
                if (ImGui::Checkbox("##SphereLightActive", &active))
                {
                    component.SetActive(active);
                }
            }

            // Shadow Type
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Shadow Type");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("What type of shadow will be cast?");
                
                ImGui::TableSetColumnIndex(1);
                
                int current_shadow_type = static_cast<int>(component.GetShadowType());
                if (ImGui::Combo("##SphereShadowTypeCombo", &current_shadow_type, s_ShadowTypeStrings.data(), static_cast<int>(s_ShadowTypeStrings.size())))
                {
                    component.SetShadowType(static_cast<ShadowType>(current_shadow_type));
                }
            }

            // Light Colour
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Colour");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The colour of the light.");
                
                ImGui::TableSetColumnIndex(1);

                auto colour = component.GetColour();
                if (ImGui::ColorEdit4("##SphereLightColour", glm::value_ptr(colour)))
                {
                    component.SetColour(colour);
                }
            }

            // Radius
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Radius");
                
                ImGui::TableSetColumnIndex(1);

                auto radius = component.GetRadius();
                if (ImGui::DragFloat("##SphereLightRadius", &radius, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetRadius(radius);
                }
            }
            
            // Intensity
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Intensity");
                
                ImGui::TableSetColumnIndex(1);

                auto intensity = component.GetIntensity();
                if (ImGui::DragFloat("##SphereLightIntensity", &intensity, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetIntensity(intensity);
                }
            }            
            
            ImGui::EndTable();
        }
    }

    static void DrawConeLightComponent(ConeLightComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##ConeLightComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Active
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Active");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Light On?");
                
                ImGui::TableSetColumnIndex(1);

                auto active = component.GetActive();
                if (ImGui::Checkbox("##ConeLightActive", &active))
                {
                    component.SetActive(active);
                }
            }

            // Shadow Type
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Shadow Type");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("What type of shadow will be cast?");
                
                ImGui::TableSetColumnIndex(1);
                
                int current_shadow_type = static_cast<int>(component.GetShadowType());
                if (ImGui::Combo("##ConeShadowTypeCombo", &current_shadow_type, s_ShadowTypeStrings.data(), static_cast<int>(s_ShadowTypeStrings.size())))
                {
                    component.SetShadowType(static_cast<ShadowType>(current_shadow_type));
                }
            }

            // Light Colour
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Colour");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The colour of the light.");
                
                ImGui::TableSetColumnIndex(1);

                auto colour = component.GetColour();
                if (ImGui::ColorEdit4("##ConeLightColour", glm::value_ptr(colour)))
                {
                    component.SetColour(colour);
                }
            }

            // Angle
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Angle");
                
                ImGui::TableSetColumnIndex(1);

                auto angle = component.GetAngle();
                if (ImGui::DragFloat("##ConeLightAngle", &angle, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetAngle(angle);
                }
            }
            
            // Range
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Range");
                
                ImGui::TableSetColumnIndex(1);

                auto range = component.GetRange();
                if (ImGui::DragFloat("##ConeLightRange", &range, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetRange(range);
                }
            }   

            // Intensity
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Intensity");
                
                ImGui::TableSetColumnIndex(1);

                auto intensity = component.GetIntensity();
                if (ImGui::DragFloat("##ConeLightIntensity", &intensity, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetIntensity(intensity);
                }
            }           
            
            ImGui::EndTable();
        }
    }

    static void DrawDirectionalLightComponent(DirectionalLightComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##DirectionalLightComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Active
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Active");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("Is Light On?");
                
                ImGui::TableSetColumnIndex(1);

                auto active = component.GetActive();
                if (ImGui::Checkbox("##DirectionalLightActive", &active))
                {
                    component.SetActive(active);
                }
            }

            // Shadow Type
            { 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Shadow Type");

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("What type of shadow will be cast?");
                
                ImGui::TableSetColumnIndex(1);
                
                int current_shadow_type = static_cast<int>(component.GetShadowType());
                if (ImGui::Combo("##DirectionalShadowTypeCombo", &current_shadow_type, s_ShadowTypeStrings.data(), static_cast<int>(s_ShadowTypeStrings.size())))
                {
                    component.SetShadowType(static_cast<ShadowType>(current_shadow_type));
                }
            }

            // Light Colour
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Colour");
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                    ImGui::SetTooltip("The colour of the light.");
                
                ImGui::TableSetColumnIndex(1);

                auto colour = component.GetColour();
                if (ImGui::ColorEdit4("##DirectionalLightColour", glm::value_ptr(colour)))
                {
                    component.SetColour(colour);
                }
            }

            // Intensity
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Intensity");
                
                ImGui::TableSetColumnIndex(1);

                auto intensity = component.GetIntensity();
                if (ImGui::DragFloat("##DirectionalLightIntensity", &intensity, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.SetIntensity(intensity);
                }
            }           
            
            ImGui::EndTable();
        }
    }

    static void DrawRigidbodyComponent(RigidbodyComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##RigidbodyComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Mass
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Mass");
                
                ImGui::TableSetColumnIndex(1);

                auto mass = component.GetRigid()->GetMass();
                if (ImGui::DragFloat("##RigidbodyMass", &mass, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetRigid()->SetMass(mass);
                }
            }
            
            // Drag
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Drag");
                
                ImGui::TableSetColumnIndex(1);

                auto drag = component.GetRigid()->GetDrag();
                if (ImGui::DragFloat("##RigidbodyDrag", &drag, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetRigid()->SetDrag(drag);
                }
            }
            
            // Angular Drag
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Angular Drag");
                
                ImGui::TableSetColumnIndex(1);

                auto angular_drag = component.GetRigid()->GetAngularDrag();
                if (ImGui::DragFloat("##RigidbodyAngularDrag", &angular_drag, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetRigid()->SetAngularDrag(angular_drag);
                }
            }
            
            // Use Gravity
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Gravity");
                
                ImGui::TableSetColumnIndex(1);

                auto gravity = component.GetRigid()->IsUsingGravity();
                if (ImGui::Checkbox("##RigidbodyGravity", &gravity))
                {
                    component.GetRigid()->SetUsingGravity(gravity);
                }
            }
            
            // Kinematic
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Kinematic");
                
                ImGui::TableSetColumnIndex(1);

                auto kinematic = component.GetRigid()->IsKinematic();
                if (ImGui::Checkbox("##RigidbodyKinematic", &kinematic))
                {
                    component.GetRigid()->SetKinematic(kinematic);
                }
            }
            
            // Auto Centre of Mass
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Auto Centre of Mass");
                
                ImGui::TableSetColumnIndex(1);

                auto auto_centre_of_mass = component.GetRigid()->IsUsingAutoCentreMass();
                if (ImGui::Checkbox("##RigidbodyAutoCentreMass", &auto_centre_of_mass))
                {
                    component.GetRigid()->SetUsingAutoCentreMass(auto_centre_of_mass);
                }

                if (!auto_centre_of_mass)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    ImGui::TextWrapped("Centre of Mass");
                    
                    ImGui::TableSetColumnIndex(1);

                    auto manual_com = component.GetRigid()->GetCentreOfMassIfNotAuto();
                    if (ImGui::DragFloat3("##RigidbodyCentreMass", glm::value_ptr(manual_com), 0.01f))
                    {
                        component.GetRigid()->SetCentreOfMassIfNotAuto(manual_com);
                    }
                }
            }

            // Position Constraint
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Position Constraint");
                
                ImGui::TableSetColumnIndex(1);

                bool constraint_modified = false;
                auto position_constraint = component.GetRigid()->GetPositionConstraint();

                ImGui::TextUnformatted("X:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyPositionConstraintX", &position_constraint[0]))
                {
                    constraint_modified = true;
                }
                ImGui::SameLine();
                
                ImGui::TextUnformatted("Y:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyPositionConstraintY", &position_constraint[1]))
                {
                    constraint_modified = true;
                }
                ImGui::SameLine();
                
                ImGui::TextUnformatted("Z:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyPositionConstraintZ", &position_constraint[2]))
                {
                    constraint_modified = true;
                }

                if (constraint_modified)
                    component.GetRigid()->SetPositionConstraint(position_constraint);
            }

            // Rotation Constraint
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Rotation Constraint");
                
                ImGui::TableSetColumnIndex(1);

                bool constraint_modified = false;
                auto rotation_constraint = component.GetRigid()->GetRotationConstraint();

                ImGui::TextUnformatted("X:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyRotationConstraintX", &rotation_constraint[0]))
                {
                    constraint_modified = true;
                }
                ImGui::SameLine();
                
                ImGui::TextUnformatted("Y:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyRotationConstraintY", &rotation_constraint[1]))
                {
                    constraint_modified = true;
                }
                ImGui::SameLine();
                
                ImGui::TextUnformatted("Z:");
                ImGui::SameLine();
                if (ImGui::Checkbox("##RigidbodyRotationConstraintZ", &rotation_constraint[2]))
                {
                    constraint_modified = true;
                }

                if (constraint_modified)
                    component.GetRigid()->SetRotationConstraint(rotation_constraint);
            }

            ImGui::EndTable();
        }
    }

    static void DrawBoxColliderComponent(BoxColliderComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##BoxColliderComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Is Trigger
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Is Trigger");
                
                ImGui::TableSetColumnIndex(1);

                auto is_trigger = component.GetShape()->IsTrigger();
                if (ImGui::Checkbox("##BoxColliderTrigger", &is_trigger))
                {
                    component.GetShape()->SetIsTrigger(is_trigger);
                }
            }
            
            // Centre
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Centre");
                
                ImGui::TableSetColumnIndex(1);

                auto centre = component.GetShape()->GetCentre();
                if (ImGui::DragFloat3("##BoxColliderCentre", glm::value_ptr(centre), 0.01f, 0.0f, 0.0f, "%.2f"))
                {
                    component.GetShape()->SetCentre(centre);
                }
            }
            
            // Half Extent
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Half Extent");
                
                ImGui::TableSetColumnIndex(1);

                auto half_extent = component.GetShape()->GetHalfExtent();
                if (ImGui::DragFloat3("##BoxColliderHalfExtent", glm::value_ptr(half_extent), 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetShape()->SetHalfExtent(half_extent);
                }
            }
            
            ImGui::EndTable();
        }
    }

    static void DrawSphereColliderComponent(SphereColliderComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##SphereColliderComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Is Trigger
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Is Trigger");
                
                ImGui::TableSetColumnIndex(1);

                auto is_trigger = component.GetShape()->IsTrigger();
                if (ImGui::Checkbox("##SphereColliderTrigger", &is_trigger))
                {
                    component.GetShape()->SetIsTrigger(is_trigger);
                }
            }
            
            // Centre
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Centre");
                
                ImGui::TableSetColumnIndex(1);

                auto centre = component.GetShape()->GetCentre();
                if (ImGui::DragFloat3("##SphereColliderCentre", glm::value_ptr(centre), 0.01f, 0.0f, 0.0f, "%.2f"))
                {
                    component.GetShape()->SetCentre(centre);
                }
            }
            
            // Radius
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Radius");
                
                ImGui::TableSetColumnIndex(1);

                float radius = component.GetShape()->GetRadius();
                if (ImGui::DragFloat("##SphereColliderRadius", &radius, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetShape()->SetRadius(radius);
                }
            }
            
            ImGui::EndTable();
        }
    }

    static void DrawCapsuleColliderComponent(CapsuleColliderComponent& component)
    {
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.409f, 0.409f, 0.409f, 1.000f));
        if (ImGui::BeginTable("##CapsuleColliderComponentTable", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::PopStyleColor();
            
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            // Is Trigger
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Is Trigger");
                
                ImGui::TableSetColumnIndex(1);

                auto is_trigger = component.GetShape()->IsTrigger();
                if (ImGui::Checkbox("##CapsuleColliderTrigger", &is_trigger))
                {
                    component.GetShape()->SetIsTrigger(is_trigger);
                }
            }
            
            // Centre
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Centre");
                
                ImGui::TableSetColumnIndex(1);

                auto centre = component.GetShape()->GetCentre();
                if (ImGui::DragFloat3("##CapsuleColliderCentre", glm::value_ptr(centre), 0.01f, 0.0f, 0.0f, "%.2f"))
                {
                    component.GetShape()->SetCentre(centre);
                }
            }
            
            // Radius
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Radius");
                
                ImGui::TableSetColumnIndex(1);

                float radius = component.GetShape()->GetRadius();
                if (ImGui::DragFloat("##CapsuleColliderRadius", &radius, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetShape()->SetRadius(radius);
                }
            }
            
            // Half Height
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Half Height");
                
                ImGui::TableSetColumnIndex(1);

                float half_height = component.GetShape()->GetHalfHeight();
                if (ImGui::DragFloat("##CapsuleColliderHalfHeight", &half_height, 0.01f, 0.0f, FLT_MAX, "%.2f"))
                {
                    component.GetShape()->SetHalfHeight(half_height);
                }
            }
            
            ImGui::EndTable();
        }
    }

    static void DrawConvexMeshColliderComponent(ConvexMeshColliderComponent& component)
    {

    }

    static void DrawHeightFieldColliderComponent(HeightFieldColliderComponent& component)
    {

    }

    static void DrawTriangleMeshColliderComponent(TriangleMeshColliderComponent& component)
    {

    }

#pragma endregion

#pragma region General Methods

#define EMPLACE_COMPONENT_DRAW_FUNCTIONS(T)                                         \
    m_ComponentDrawFunctions.emplace(Entity::GetBlankComponent<T>().GetType(),      \
        [](ComponentBase& base) \
        {                                                                           \
            Draw##T(static_cast<T&>(base));                                         \
        });
        
    InspectorPanel::InspectorPanel()
    {
        EXPAND_COMPONENTS(EMPLACE_COMPONENT_DRAW_FUNCTIONS);
    }

    void InspectorPanel::OnRenderGUI()
    {
        if (m_Active)
        {
            if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
            {
                switch(m_ContextType)
                {
                    case InspectorContextType_Entity:
                    {
                        DrawEntityProperties();
                        break;
                    }
                    case InspectorContextType_AssetProperties:
                    {

                        break;
                    }
                }
            }
            ImGui::End();
        }
    }

#pragma endregion

#pragma region DrawEntityProperties

    void InspectorPanel::DrawEntityHeader(const Entity& entity)
    {
        auto& meta_component = entity.GetComponent<MetaComponent>();
        auto& transform_component = entity.GetComponent<TransformComponent>();

		ImGui::Dummy({ 0.0f, 5.0f });

        if (ImGui::BeginTable("EntityHeaderMetaTable", 2))
        {
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);

            // Name
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Entity Name");
                
                ImGui::TableSetColumnIndex(1);

                char label_buf[256] = {};
                strncpy(label_buf, meta_component.GetName().c_str(), sizeof(label_buf) - 1);
                ImGui::InputText("##EntityName", label_buf, sizeof(label_buf), ImGuiInputTextFlags_EnterReturnsTrue);

                // Capture Name Modification
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    meta_component.SetName(label_buf);
                }
            }
            
            // GUID
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Entity GUID");
                
                ImGui::TableSetColumnIndex(1);

                char guid_buf[21] = {};
                std::string guid_str = std::to_string(meta_component.GetEntityGUID());
                strncpy(guid_buf, guid_str.c_str(), sizeof(guid_buf) - 1);
            }


            ImGui::EndTable();
        }
    
		ImGui::Dummy({ 0.0f, 5.0f });
        ImGui::Separator();
		ImGui::Dummy({ 0.0f, 5.0f });

        if (ImGui::BeginTable("EntityHeaderTransformTable", 2))
        {
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);

                
            static std::unordered_map<GUID, bool> s_GlobalTransformMap = {};
            GUID entity_guid = entity.GetGUID();
                
            if (!s_GlobalTransformMap.contains(entity_guid))
                s_GlobalTransformMap.emplace(entity_guid, false);

            float col_width;

            {
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Transform Mode");
                
                ImGui::TableSetColumnIndex(1);

                col_width = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

                static std::vector<const char*> s_TransformModes = 
                {
                    "Local",
                    "Global"
                };

                int current_mode = static_cast<int>(s_GlobalTransformMap[entity_guid]);

                ImGui::TextUnformatted(" "); // Dummy
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::Combo("##TransformModeCombo", &current_mode, s_TransformModes.data(), static_cast<int>(s_TransformModes.size())))
                {
                    s_GlobalTransformMap[entity_guid] = current_mode;
                }
            }

            // Position
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Position");
                
                ImGui::TableSetColumnIndex(1);
                
                glm::vec3 value = s_GlobalTransformMap[entity_guid] ? transform_component.GetGlobalPosition() : transform_component.GetLocalPosition();
                bool value_modified = false;

                ImGui::Text("X");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##PositionX", &value.x, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Y");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##PositionY", &value.y, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Z");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##PositionZ", &value.z, 0.01f, 0, 0, "%.2f")) value_modified = true;

                if (value_modified) 
                {
                    transform_component.SetPosition(value, s_GlobalTransformMap[entity_guid]);
                }
            }
            
            // Rotation
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Rotation");
                
                ImGui::TableSetColumnIndex(1);
                
                glm::vec3 rotation_cache = s_GlobalTransformMap[entity_guid] ? transform_component.GetGlobalOrientationEulerHint() : transform_component.GetLocalOrientationEulerHint();
                glm::vec3 value = rotation_cache;

                bool value_modified = false;

                ImGui::Text("X");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##RotationX", &value.x, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Y");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##RotationY", &value.y, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Z");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##RotationZ", &value.z, 0.01f, 0, 0, "%.2f")) value_modified = true;
                
                if (value_modified) 
                {
                    transform_component.RotateEuler(value - rotation_cache, s_GlobalTransformMap[entity_guid]);
                }
            }
            
            // Scale
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::TextWrapped("Scale");
                
                ImGui::TableSetColumnIndex(1);
                
                glm::vec3 value = s_GlobalTransformMap[entity_guid] ? transform_component.GetGlobalScale() : transform_component.GetLocalScale();
                bool value_modified = false;

                ImGui::Text("X");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##ScaleX", &value.x, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Y");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##ScaleY", &value.y, 0.01f, 0, 0, "%.2f")) value_modified = true;

                ImGui::SameLine();
                ImGui::Text("Z");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(col_width / 3);
                if (ImGui::DragFloat("##ScaleZ", &value.z, 0.01f, 0, 0, "%.2f")) value_modified = true;

                if (value_modified) 
                {
                    transform_component.SetScale(value, s_GlobalTransformMap[entity_guid]);
                }
            }
            
            ImGui::EndTable();
        }
    
		ImGui::Dummy({ 0.0f, 5.0f });
        ImGui::Separator();
		ImGui::Dummy({ 0.0f, 5.0f });
    }

    void InspectorPanel::DrawEntityScripts(const Entity &entity)
    {
        MetaComponent& meta_component = entity.GetComponent<MetaComponent>();

        ScriptManager* script_manager = Application::GetScriptManager();
        const auto& script_classes = script_manager->GetScriptClassInfoMap();

        std::vector<const char*> script_name_selections;
        script_name_selections.reserve(script_classes.size() + 1);

        script_name_selections.push_back("None");
        for (const auto& class_info : script_classes)
        {
            script_name_selections.push_back(class_info.first.c_str());
        }

        float button_width = ImGui::CalcTextSize("Add Script").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float available_width = ImGui::GetContentRegionAvail().x;
        float offset = (available_width - button_width) * 0.5f;

        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (ImGui::Button("Add Script"))
        {
            meta_component.AddScript("");
        }

        const auto& scripts = meta_component.GetScripts();
        for (size_t i = 0; i < scripts.size(); ++i)
        {
            ImGui::PushID(i);
            const auto& [script_name, script_active] = scripts[i];
            std::string label = script_name.empty() ? "Empty Script" : script_name;
            if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
            {
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Remove Script"))
                    {
                        Application::Get()->SubmitToMainThread([entity, i]()
                        {
                            if (!entity)
                                return;
                                
                            entity.GetComponent<MetaComponent>().RemoveScript(i);
                        });
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginTable("ScriptTable", 2))
                {
                    
                    ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);

                    // Script Fields - need three version
                    // 1. When Running
                    // 2. When Not Running -> Manual Value Set, show value set in editor / serialised in entity in .scene file
                    // 3. When Not Running -> No Manual Value Set, show default script value
                    // for (const auto& field : script_fields)
                    
                    // Active Button
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        ImGui::TextWrapped("Active");
                        
                        ImGui::TableSetColumnIndex(1);
                        
                        bool active = script_active;
                        if (ImGui::Checkbox("##ScriptActive", &active))
                        {
                            meta_component.UpdateScriptActiveState(i, active);
                        }
                    }

                    // Script Selection Combo
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        ImGui::TextWrapped("Selected Script");
                        
                        ImGui::TableSetColumnIndex(1);
                        
                        int current_script = 0;
                        if (!script_name.empty())
                        {
                            auto it = std::find_if(script_name_selections.begin(), script_name_selections.end(),
                                                [&](const char* a) { return std::strcmp(a, script_name.c_str()) == 0; });
                            if (it != script_name_selections.end())
                                current_script = static_cast<int>(std::distance(script_name_selections.begin(), it));
                        }

                        if (ImGui::Combo("##ScriptSelectionCombo", &current_script, script_name_selections.data(), static_cast<int>(script_name_selections.size())))
                        {
                            meta_component.UpdateScriptNameAtIndex(i, script_name_selections[current_script]);

                            script_manager->RemoveScriptInstance(script_name_selections[current_script], entity.GetGUID(), i);
                            script_manager->RemoveScriptFieldInstance(script_name_selections[current_script], entity.GetGUID(), i);
                        }
                    }

                    // TODO: Draw Script Fields

                    ImGui::EndTable();
                }

                ImGui::TreePop();
            }
            else
            {
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Remove Script"))
                    {
                        Application::Get()->SubmitToMainThread([entity, i]()
                        {
                            if (!entity)
                                return;
                                
                            entity.GetComponent<MetaComponent>().RemoveScript(i);
                        });
                    }
                    ImGui::EndPopup();
                }
            }

            if (i + 1 != scripts.size())
            {
                ImGui::Dummy({ 0.0f, 2.5f });
                ImGui::Separator();
                ImGui::Dummy({ 0.0f, 2.5f });
            }

            ImGui::PopID();
        }
    
        // -----------------------------------------------------------
        // TODO: TO BE USED FOR DROPPING COMPONENTS ONTO SCRIPT FIELDS
        // -----------------------------------------------------------
        // if (ImGui::BeginDragDropTarget())
        // {
        //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_COMPONENT"))
        //     {
        //         struct ComponentPayload
        //         {
        //             Entity entity;
        //             ComponentType type;
        //         };
        //         ComponentPayload dropped_component = *(ComponentPayload*)payload->Data;

        //         BC_CORE_INFO("{}, {}", dropped_component.entity.GetName(), Util::ComponentTypeToString(dropped_component.type));
        //     }

        //     ImGui::EndDragDropTarget();
        // }

    }

    static std::unordered_map<ComponentType, std::string> s_ComponentNameMap = {};

    template<typename... Component>
    static void DrawEntityContextMenuHelper(Entity& entity)
    {
        ([&]()
            {
                // Don't Add Transform or MetaComponent's
                if constexpr (std::is_same_v<Component, TransformComponent> || std::is_same_v<Component, MetaComponent>)
                    return;
                
                ComponentType type = Entity::GetBlankComponent<Component>().GetType();

                if (!s_ComponentNameMap.contains(type))
                    s_ComponentNameMap[type] = Util::ComponentTypeToString(type);

                if (ImGui::MenuItem(std::string("Add " + s_ComponentNameMap[type]).c_str()))
                {
                    entity.AddComponent<Component>();
                }

                switch (type)
                {
                    case ComponentType::CameraComponent:
                    case ComponentType::AudioEmitterComponent:
                    case ComponentType::SkinnedMeshRendererComponent:
                    case ComponentType::AnimatorComponent:
                    case ComponentType::DirectionalLightComponent:
                    case ComponentType::RigidbodyComponent:
                    {
                        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ImGui::Dummy({ 0.0f, 2.5f });
                        ImGui::Separator();
                        ImGui::Dummy({ 0.0f, 2.5f });
                        ImGui::PopStyleColor();
                        break;
                    }
                    default: break;
                }
                
            }(), ...);
    }

    template<typename... Component>
    static void DrawEntityContextMenuHelper(Util::ComponentGroup<Component...>, Entity& entity)
    {
        DrawEntityContextMenuHelper<Component...>(entity);
    }

    template<typename... Component>
    static void DrawComponentHelper(Entity& entity, std::unordered_map<ComponentType, std::function<void(ComponentBase&)>>& draw_functions)
    {
        ([&]()
            {
                // Don't Draw Transform or MetaComponent's
                if constexpr (std::is_same_v<Component, TransformComponent> || std::is_same_v<Component, MetaComponent>)
                    return;

                if (!entity.HasComponent<Component>())
                    return;

                ComponentType type = Entity::GetBlankComponent<Component>().GetType();

                if (!s_ComponentNameMap.contains(type))
                    s_ComponentNameMap[type] = Util::ComponentTypeToString(type);

                // TODO: Make a better tree node with the widgets I need, e.g., Active, Open Arrow, ... Settings, More Aesthetic Component title
                if (ImGui::TreeNodeEx(("##ComponentNode_" + s_ComponentNameMap.at(type)).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, "%s", s_ComponentNameMap.at(type).c_str()))
                {
                    if (ImGui::BeginDragDropSource())
                    {
                        struct
                        {
                            Entity entity;
                            ComponentType type;
                        } component_payload = { entity, type };
                        ImGui::SetDragDropPayload("ENTITY_COMPONENT", &component_payload, sizeof(component_payload));
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Remove Component"))
                        {
                            Application::Get()->SubmitToMainThread([guid = entity.GetGUID()]() 
                            {
                                Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(guid);
                                if (!entity)
                                    return;
                                    
                                entity.RemoveComponent<Component>();
                            });
                        }
                        ImGui::EndPopup();
                    }

                    if (draw_functions.contains(type))
                        draw_functions.at(type)(entity.GetComponent<Component>());

                    ImGui::TreePop();
                }
                else
                {
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Remove Component"))
                        {
                            Application::Get()->SubmitToMainThread([guid = entity.GetGUID()]() 
                            {
                                Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(guid);
                                if (!entity)
                                    return;
                                    
                                entity.RemoveComponent<Component>();
                            });
                        }
                        ImGui::EndPopup();
                    }
                }

            }(), ...);
    }

    template<typename... Component>
    static void DrawComponentGroupHelper(Util::ComponentGroup<Component...>, Entity& entity, std::unordered_map<ComponentType, std::function<void(ComponentBase&)>>& draw_functions)
    {
        DrawComponentHelper<Component...>(entity, draw_functions);
    }

    void InspectorPanel::DrawEntityProperties()
    {
        Entity entity = m_Selection.GetSelection<Entity>();
        if (!entity)
            return;
        
        ImGui::PushID(entity.GetGUID());
        DrawEntityHeader(entity);

        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.448f, 0.448f, 0.448f, 0.448f));
        if (ImGui::BeginTabBar("EntityInspectorTabs"))
        {
            if (ImGui::BeginTabItem("Components"))
            {
                ImGui::BeginChild("##Component");

                ImGui::Dummy({ 0.0f, 5.0f });
                DrawComponentGroupHelper(Util::AllComponents{}, entity, m_ComponentDrawFunctions);

                ImGui::InvisibleButton("#BlankSpace", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y));

                if (ImGui::BeginPopupContextItem())
                {
                    DrawEntityContextMenuHelper(Util::AllComponents{}, entity);

                    ImGui::EndPopup();
                }
                
                ImGui::EndChild();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Scripts"))
            {
                ImGui::BeginChild("##Scripts");
                ImGui::Dummy({ 0.0f, 5.0f });
                DrawEntityScripts(entity);
                ImGui::EndChild();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::PopStyleColor();

        ImGui::PopID();
    }

#pragma endregion

#pragma region DrawAssetProperties

    void InspectorPanel::DrawAssetProperties()
    {
    }
    
#pragma endregion

}