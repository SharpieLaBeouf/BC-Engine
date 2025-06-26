#include "BC_PCH.h"
#include "InspectorPanel.h"

namespace BC
{

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
            // Fixed width for column 0
            ImGui::TableSetupColumn("##DataName", ImGuiTableColumnFlags_WidthFixed, 100.0f); // adjust width as needed
            // Stretch remaining space for column 1
            ImGui::TableSetupColumn("##DataValue", ImGuiTableColumnFlags_WidthStretch);
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::TextWrapped("Clear Colour");
            
            ImGui::TableSetColumnIndex(1);

            auto clear_colour = component.GetClearColour();
            if (ImGui::ColorEdit4("##ClearColour", glm::value_ptr(clear_colour)))
            {
                component.SetClearColour(clear_colour);
            }
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::TextWrapped("Viewport");
            
            ImGui::TableSetColumnIndex(1);

            auto view_port = component.GetViewport();
            if (ImGui::DragFloat4("##Viewport", glm::value_ptr(view_port), 0.01f, 0.0f, 1.0f))
            {
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

    }

    static void DrawSkinnedMeshRendererComponent(SkinnedMeshRendererComponent& component)
    {

    }

    static void DrawSimpleAnimationComponent(SimpleAnimationComponent& component)
    {

    }

    static void DrawAnimatorComponent(AnimatorComponent& component)
    {

    }

    static void DrawSphereLightComponent(SphereLightComponent& component)
    {

    }

    static void DrawConeLightComponent(ConeLightComponent& component)
    {

    }

    static void DrawDirectionalLightComponent(DirectionalLightComponent& component)
    {

    }

    static void DrawRigidbodyComponent(RigidbodyComponent& component)
    {

    }

    static void DrawPlaneCollider(PlaneCollider& component)
    {

    }

    static void DrawBoxColliderComponent(BoxColliderComponent& component)
    {

    }

    static void DrawSphereColliderComponent(SphereColliderComponent& component)
    {

    }

    static void DrawCapsuleColliderComponent(CapsuleColliderComponent& component)
    {

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

    void InspectorPanel::DrawEntityHeader()
    {

    }

    template<typename... Component>
    static void DrawComponentHelper(const Entity& entity, std::unordered_map<ComponentType, std::function<void(ComponentBase&)>>& draw_functions)
    {
        ([&]()
            {
                // Don't Draw Transform or MetaComponent's
                if constexpr (std::is_same_v<Component, TransformComponent> || std::is_same_v<Component, MetaComponent>)
                    return;

                if (!entity.HasComponent<Component>())
                    return;

                ImGui::PushID(entity.GetGUID());
                ComponentType type = Entity::GetBlankComponent<Component>().GetType();

                static std::unordered_map<ComponentType, std::string> s_ComponentNameMap = {};
                if (!s_ComponentNameMap.contains(type))
                    s_ComponentNameMap[type] = Util::ComponentTypeToString(type);

                // TODO: Make a better tree node with the widgets I need, e.g., Active, Open Arrow, ... Settings, More Aesthetic Component title
                if (ImGui::TreeNodeEx(("##ComponentNode_" + s_ComponentNameMap.at(type)).c_str(), 0, "%s", s_ComponentNameMap.at(type).c_str()))
                {
                    if (draw_functions.contains(type))
                        draw_functions.at(type)(entity.GetComponent<Component>());

                    ImGui::TreePop();
                }
                ImGui::PopID();
            }(), ...);
    }

    template<typename... Component>
    static void DrawComponentGroupHelper(Util::ComponentGroup<Component...>, const Entity& entity, std::unordered_map<ComponentType, std::function<void(ComponentBase&)>>& draw_functions)
    {
        DrawComponentHelper<Component...>(entity, draw_functions);
    }

    void InspectorPanel::DrawEntityProperties()
    {
        const Entity& entity = m_Selection.GetSelection<Entity>();
        if (!entity)
            return;
        
        DrawEntityHeader();
        DrawComponentGroupHelper(Util::AllComponents{}, entity, m_ComponentDrawFunctions);
    }

#pragma endregion

#pragma region DrawAssetProperties

    void InspectorPanel::DrawAssetProperties()
    {
    }
    
#pragma endregion

}