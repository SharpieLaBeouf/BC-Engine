#pragma once
#include "BC.h"

#include "PanelBase.h"

namespace BC
{

    class HierarchyPanel : public IEditorPanel
    {
        static ImVec2 s_DraggingSourceMin;
        static ImVec2 s_DraggingSourceMax;
        static Entity s_SourceDragDropEntity;
        static Entity s_TargetDragDropEntity;

    public:

        HierarchyPanel() = default;
        ~HierarchyPanel() = default;

        PanelType GetType() const override { return PanelType_Hierarchy; }

        void OnUpdate() override { }
        void OnRenderGUI() override;

    private:

        void DrawEntityRecursive(const std::shared_ptr<Scene>& scene, const Entity& current_entity, const std::unordered_map<GUID, Entity>& all_entities);
        void DrawScene(std::shared_ptr<Scene> scene);

        friend class EditorLayer;

    };

}