#pragma once

#include "PanelBase.h"

namespace BC
{

    class StatisticsPanel : public IEditorPanel
    {

    public:

        StatisticsPanel() = default;
        ~StatisticsPanel() = default;

        PanelType GetType() const override { return PanelType_Statistics; }

        void OnUpdate() override {}
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