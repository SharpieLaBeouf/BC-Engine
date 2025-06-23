#pragma once

#include "PanelBase.h"

namespace BC
{

    class AnimatorPanel : public IEditorPanel
    {

    public:

        AnimatorPanel() = default;
        ~AnimatorPanel() = default;

        PanelType GetType() const override { return PanelType_AnimatorNodeGraph; }

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