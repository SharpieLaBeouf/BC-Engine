#pragma once

#include "PanelBase.h"

namespace BC
{

    class HumanoidConfigPanel : public IEditorPanel
    {

    public:

        HumanoidConfigPanel() = default;
        ~HumanoidConfigPanel() = default;

        PanelType GetType() const override { return PanelType_HumanoidConfig; }

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