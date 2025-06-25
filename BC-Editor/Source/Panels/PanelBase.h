#pragma once

#include "Selection/SelectionContext.h"

#include <string>
#include <cstdint>

#include <imgui.h>

namespace BC
{

    using PanelType = uint8_t;
    enum : PanelType
    {
        PanelType_Unknown,

        PanelType_Inspector,
        PanelType_Hierarchy,
        PanelType_ContentBrowser,
        PanelType_AssetRegistry,

        PanelType_SceneViewport,

        PanelType_Console,
        PanelType_Profiler,
        PanelType_Statistics,

        PanelType_HumanoidConfig,
        PanelType_AnimatorNodeGraph,

        PanelType_ProjectConfig,
        PanelType_SceneManagerConfig,
        PanelType_SceneConfig
    };

    namespace Util
    {
        const char* PanelTypeToString(PanelType type);
        PanelType PanelTypeFromString(const std::string& str);
    }

    class EditorLayer;

    struct IEditorPanel
    {

    public:

        IEditorPanel() = default;
        virtual ~IEditorPanel() = default;
        virtual PanelType GetType() const = 0;

        // Delete Copies
        IEditorPanel(const IEditorPanel&) = delete;
        IEditorPanel& operator=(const IEditorPanel&) = delete;

        // Default Moves
        IEditorPanel(IEditorPanel&&) = default;
        IEditorPanel& operator=(IEditorPanel&&) = default;

        bool GetActive() const { return m_Active; };
        void SetActive(bool active) { m_Active = active; };

        void SetEditorLayerReference(EditorLayer* editor_layer) { m_EditorLayer = editor_layer; }

        virtual void OnUpdate() { }
        virtual void OnRenderGUI() = 0;

        SelectionContext* GetSelectionContext() { return &m_Selection; }

    protected:

        bool m_Active = true;
        EditorLayer* m_EditorLayer = nullptr;
        SelectionContext m_Selection;

        friend class EditorLayer;

    };

}