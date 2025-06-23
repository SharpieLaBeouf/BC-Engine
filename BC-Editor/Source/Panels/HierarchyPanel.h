#pragma once

#include "PanelBase.h"

namespace BC
{

    class HierarchyPanel : public IEditorPanel
    {

    public:

        HierarchyPanel() = default;
        ~HierarchyPanel() = default;

        PanelType GetType() const override { return PanelType_Hierarchy; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {

                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

}