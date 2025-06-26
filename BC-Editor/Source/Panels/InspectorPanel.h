#pragma once
#include "BC.h"

#include "PanelBase.h"

namespace BC
{

    using InspectorContextType = uint8_t;
    enum : InspectorContextType
    {
        InspectorContextType_None,
        InspectorContextType_Entity,
        InspectorContextType_AssetProperties
    };
    

    class InspectorPanel : public IEditorPanel
    {

    public:

        InspectorPanel();
        ~InspectorPanel() = default;

        PanelType GetType() const override { return PanelType_Inspector; }

        void OnUpdate() override { }
        void OnRenderGUI() override;

        void SetContextEntity(const Entity& entity) { m_Selection.SetSelection(entity); m_ContextType = InspectorContextType_Entity; }
        void SetContextAssetProperties(const AssetHandle& asset_handle) { m_Selection.SetSelection(asset_handle); m_ContextType = InspectorContextType_AssetProperties; }

    private:

        void DrawEntityHeader();
        void DrawEntityProperties();
        void DrawAssetProperties();

    private:

        std::unordered_map<ComponentType, std::function<void(ComponentBase&)>> m_ComponentDrawFunctions;

        InspectorContextType m_ContextType = InspectorContextType_None;

        friend class EditorLayer;

    };

}