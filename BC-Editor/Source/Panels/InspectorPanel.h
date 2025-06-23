#pragma once

#include "PanelBase.h"

namespace BC
{

    class InspectorPanel : public IEditorPanel
    {

    public:

        InspectorPanel() = default;
        ~InspectorPanel() = default;

        PanelType GetType() const override { return PanelType_Inspector; }

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