#include "BC_PCH.h"
#include "HierarchyPanel.h"

namespace BC
{
    ImVec2 HierarchyPanel::s_DraggingSourceMin = {};
    ImVec2 HierarchyPanel::s_DraggingSourceMax = {};

    Entity HierarchyPanel::s_SourceDragDropEntity = {};
    Entity HierarchyPanel::s_TargetDragDropEntity = {};

    void HierarchyPanel::OnRenderGUI()
    {
        if (m_Active)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
            {
                ImGui::PopStyleVar();
                ImGui::BeginChild("HierarchyChildArea");

                ImVec2 min = ImGui::GetCursorScreenPos();
                ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetContentRegionAvail().y);
                ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 0, 0, 255));

                auto scene_manager = Application::GetProject()->GetSceneManager();
                
                for (auto& [scene_id, scene] : scene_manager->GetSceneInstances())
                {
                    DrawScene(scene);
                }
                
                DrawScene(scene_manager->GetPersistentScene());

                ImGui::EndChild();

                // This is to ensure that when we are dragging an entity, if
                // dropped on itself it won't catch the Begin/End child as its
                // target, rather will leave it in place
                ImVec2 mouse_pos = ImGui::GetMousePos();
                bool mouse_over_source = mouse_pos.x >= s_DraggingSourceMin.x &&
                                        mouse_pos.y >= s_DraggingSourceMin.y &&
                                        mouse_pos.x <= s_DraggingSourceMax.x &&
                                        mouse_pos.y <= s_DraggingSourceMax.y;

                if (!mouse_over_source)
                {
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                        {
                            Entity dropped_entity = *(Entity*)payload->Data;

                            if (dropped_entity)
                            {
                                // Store state for processing outside the render loop
                                s_SourceDragDropEntity = dropped_entity;
                                s_TargetDragDropEntity = {};
                            }
                                
                            s_DraggingSourceMin = {};
                            s_DraggingSourceMax = {};
                        }

                        ImGui::EndDragDropTarget();
                    }
                }
            }
            ImGui::End();
        }
    }

    static int s_ChildDepth = -1;
    void HierarchyPanel::DrawEntityRecursive(const std::shared_ptr<Scene>& scene, const Entity& current_entity, const std::unordered_map<GUID, Entity>& all_entities)
    {
        s_ChildDepth++;
        std::string entity_label = current_entity.GetName();

        ImGuiTreeNodeFlags entity_tree_node_flags = 
            ImGuiTreeNodeFlags_SpanAvailWidth       |
            ImGuiTreeNodeFlags_OpenOnArrow          |
            ImGuiTreeNodeFlags_OpenOnDoubleClick;

        auto& meta_component = current_entity.GetComponent<MetaComponent>();
        bool has_children = meta_component.HasChildren();
        
        if (m_Selection.GetSelection<Entity>() == current_entity)
        {
            entity_tree_node_flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (has_children)
        {
            entity_tree_node_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        }
        else
        {
			entity_tree_node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen;
        }

        ImVec2 drop_pos_min = ImGui::GetCursorScreenPos();
        ImVec2 drop_pos_max = ImVec2(drop_pos_min.x + ImGui::GetContentRegionAvail().x, drop_pos_min.y + 4.0f);

        ImGui::PushID(current_entity.GetGUID());

        // Reserve space and create the drop target

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, {0.0f, 0.0f});
        ImGui::InvisibleButton("DropLine", ImVec2(-1, 2.0f));
        ImGui::PopStyleVar(3);
        if (s_ChildDepth != 0)
        {
            if (ImGui::BeginDragDropTarget())
            {                
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                {
                    Entity dropped_entity = *(Entity*)payload->Data;

                    if (dropped_entity && dropped_entity != current_entity)
                    {
                        Application::Get()->SubmitToMainThread([dropped_entity, current_entity]()
                        {
                            const Entity& parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(current_entity.GetComponent<MetaComponent>().GetParentGUID());
                            if (!parent_entity)
                                return;

                            // Reparent the dropped entity to the same parent as the current one
                            dropped_entity.GetComponent<MetaComponent>().AttachParent(parent_entity.GetGUID());

                            // Reorder the dropped entity to appear just before the current one in the parent's child list
                            auto& children = parent_entity.GetComponent<MetaComponent>().m_Children;
                            GUID dropped_guid = dropped_entity.GetGUID();
                            GUID current_guid = current_entity.GetGUID();

                            // Remove if already present to avoid duplication
                            auto dropped_it = std::find(children.begin(), children.end(), dropped_guid);
                            BC_ASSERT(dropped_it != children.end(), "HierarchyPanel::DrawEntityRecursive: We Attached To Parent, But Dropped GUID Not Found In Parent->Children Vector.");

                            if (dropped_it != children.end())
                                children.erase(dropped_it);

                            // Insert before the current entity
                            auto insert_pos = std::find(children.begin(), children.end(), current_guid);
                            children.insert(insert_pos, dropped_guid);
                        });
                    }
                    s_DraggingSourceMin = {};
                    s_DraggingSourceMax = {};
                }

                ImGui::EndDragDropTarget();
            }
        }

        // Override horizontal position manually
        ImVec2 screen_pos = ImGui::GetCursorScreenPos();
        screen_pos.x = ImGui::GetWindowPos().x + s_ChildDepth * ImGui::GetStyle().IndentSpacing;
        ImGui::SetCursorScreenPos(screen_pos);

        int colours_pushed = 0;
        bool prefab_valid;
        if (meta_component.GetPrefabHandle() != NULL_GUID && m_Selection.GetSelection<Entity>() != current_entity)
        {
            if (!AssetManager::IsAssetHandleValid(meta_component.GetPrefabHandle()))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, { 0.4627f, 0.6275f, 0.8667f, 1.0f });
                colours_pushed++;
                prefab_valid = true;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, { 0.8196f, 0.4196f, 0.4196f, 1.0f });
                colours_pushed++;
                prefab_valid = false;
            }
        }

        bool tree_node_open = ImGui::TreeNodeEx("##EntityNode", entity_tree_node_flags, "%s", entity_label.c_str());

        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();

        if (ImGui::BeginDragDropSource())
        {
            s_DraggingSourceMin = min;
            s_DraggingSourceMax = max;

            ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &current_entity, sizeof(Entity));
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
            {
                Entity dropped_entity = *(Entity*)payload->Data;

                if (dropped_entity && dropped_entity != current_entity)
                {
                    // Store state for processing outside the render loop
                    s_SourceDragDropEntity = dropped_entity;
                    s_TargetDragDropEntity = current_entity;
                }
                s_DraggingSourceMin = {};
                s_DraggingSourceMax = {};
            }

            ImGui::EndDragDropTarget();
        }
        
        if (colours_pushed != 0)
        {
            ImGui::PopStyleColor(colours_pushed);
            if (prefab_valid && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
            {
                ImGui::SetTooltip("Prefab connection valid.");
            }
            else if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
            {
                ImGui::SetTooltip("Prefab connection invalid.");
            }
        }

        // Capture if Clicking the Label Component of TreeNode to Select This Entity
        {
            const ImGuiStyle& style = ImGui::GetStyle();
            float font_size = ImGui::GetFontSize();
            float padding_x = style.FramePadding.x;
            float text_offset_x = font_size + ((entity_tree_node_flags & ImGuiTreeNodeFlags_Framed) ? padding_x * 3.0f : padding_x * 2.0f);

            ImVec2 item_min = ImGui::GetItemRectMin();
            ImVec2 item_max = ImGui::GetItemRectMax();
            ImVec2 label_min(item_min.x + text_offset_x, item_min.y);
            ImVec2 label_max(item_max.x, item_max.y);

            if (ImGui::IsMouseHoveringRect(label_min, label_max) && ImGui::IsMouseClicked(0))
                m_Selection.SetSelection(current_entity);
        }

        ImGui::PopID();
        
        // INVISIBLE DROP LINE, BECOMES VISIBLE WHEN HOVERING OVER THIS DROP LINE - after tree node
        // if dropped s_EntityDroppedAfter = current_entity; s_DroppedEntity = dropped_entity (from pay load);

        if (!tree_node_open)
        {
            s_ChildDepth--;
            return;
        }

        if (!has_children)
        {
            s_ChildDepth--;
            return;
        }
        
        for (const auto& child_guid : current_entity.GetComponent<MetaComponent>().GetChildrenGUID())
        {
            if (!all_entities.contains(child_guid))
                continue;
            
            const Entity& child_entity = all_entities.at(child_guid);
            if (!child_entity)
                continue;
            
            DrawEntityRecursive(scene, child_entity, all_entities);
        }

        ImGui::TreePop();
        
        s_ChildDepth--;
    }

    void HierarchyPanel::DrawScene(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;

        const std::string& scene_name = scene->GetName();

        // Remove padding and set unique ID
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushID(scene_name.c_str());
        
        // Draw full-width background for scene node
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        float window_width = ImGui::GetWindowSize().x;
        float line_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2;

        ImGui::GetWindowDrawList()->AddRectFilled
        (
            ImVec2(window_pos.x, cursor_pos.y),
            ImVec2(window_pos.x + window_width, cursor_pos.y + line_height),
            IM_COL32(30, 30, 30, 255)
        );

        ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(30, 30, 30, 255));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(30, 30, 30, 255));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(30, 30, 30, 255));

        // Override horizontal position manually
        ImVec2 screen_pos = ImGui::GetCursorScreenPos();
        screen_pos.x = ImGui::GetWindowPos().x;
        ImGui::SetCursorScreenPos(screen_pos);

        // Render tree node with label hidden from ID
        bool open = ImGui::TreeNodeEx("##SceneNode", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", scene_name.c_str());

        ImGui::PopID();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        if (!open)
            return;

        auto all_entities_view = scene->GetAllEntitiesWith<MetaComponent>();

        static std::unordered_map<GUID, std::vector<Entity>> root_entities;
        static std::unordered_map<GUID, std::unordered_map<GUID, Entity>> all_entities;

        root_entities[scene->GetSceneID()].reserve(root_entities.size());
        all_entities[scene->GetSceneID()].reserve(all_entities_view.size());
        
        if ((root_entities[scene->GetSceneID()].empty() || all_entities[scene->GetSceneID()].empty()) && !all_entities_view.empty() || scene->IsHierarchyDirty())
        {
            root_entities[scene->GetSceneID()].clear();
            all_entities[scene->GetSceneID()].clear();

            for (const auto& entity_handle : all_entities_view)
            {
                auto& meta_component = all_entities_view.get<MetaComponent>(entity_handle);

                all_entities[scene->GetSceneID()][meta_component.GetEntityGUID()] = Entity(entity_handle, scene.get());
                if (!meta_component.HasParent())
                {
                    root_entities[scene->GetSceneID()].emplace_back(entity_handle, scene.get());
                }
            }
        }

        for (const auto& entity : root_entities[scene->GetSceneID()])
        {
            DrawEntityRecursive(scene, entity, all_entities[scene->GetSceneID()]);
        }

        ImGui::TreePop();

        Application::Get()->SubmitToMainThread([&]()
        {
            if (s_SourceDragDropEntity && s_SourceDragDropEntity != s_TargetDragDropEntity)
            {
                if (s_TargetDragDropEntity)
                {
                    s_SourceDragDropEntity.GetComponent<MetaComponent>().AttachParent(s_TargetDragDropEntity.GetGUID());
                }
                else
                {
                    s_SourceDragDropEntity.GetComponent<MetaComponent>().DetachParent();
                }
                s_SourceDragDropEntity = {};
                s_TargetDragDropEntity = {};
            }
        });
    }

}