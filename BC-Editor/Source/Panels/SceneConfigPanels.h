#pragma once

#include "PanelBase.h"

namespace BC
{

    class SceneManagerConfigPanel : public IEditorPanel
    {

    public:

        SceneManagerConfigPanel() = default;
        ~SceneManagerConfigPanel() = default;

        PanelType GetType() const override { return PanelType_SceneManagerConfig; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {
                    auto project = Application::GetProject();
                    auto scene_manager = project->GetSceneManager();
                    auto& scene_file_paths = scene_manager->GetSceneFilePaths();
                    GUID entry_scene = scene_manager->GetEntryScene();

                    ImGui::Text("Entry Scene:");

                    ImGui::SameLine();

                    int current_scene = 0;
                    std::vector<std::string> scene_names_str;
                    std::vector<const char*> scene_names;

                    scene_names_str.reserve(scene_file_paths.size());
                    scene_names.reserve(scene_file_paths.size());

                    for (auto it = scene_file_paths.begin(); it != scene_file_paths.end(); ++it)
                    {
                        scene_names_str.push_back(it->second.stem().string());
                        scene_names.push_back(scene_names_str.back().c_str());
                        if (it->first == entry_scene)
                            current_scene = static_cast<int>(std::distance(scene_file_paths.begin(), it));
                    }

                    if (ImGui::Combo("##EntrySceneCombo", &current_scene, scene_names.data(), static_cast<int>(scene_names.size())))
                    {
                        scene_manager->SetEntryScene(std::next(scene_file_paths.begin(), current_scene)->first);
                    }

                    // Scene List Table
                    if (ImGui::BeginTable("SceneList", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame))
                    {
                        ImGui::TableSetupColumn("Scene Name");
                        ImGui::TableSetupColumn("Scene ID");
                        ImGui::TableSetupColumn("Included");
                        ImGui::TableHeadersRow();

                        const std::filesystem::path scene_dir = project->GetDirectory() / "Scenes";

                        for (const auto& entry : std::filesystem::directory_iterator(scene_dir))
                        {
                            if (!entry.is_regular_file() || entry.path().extension() != ".scene")
                                continue;

                            std::filesystem::path relative_path = std::filesystem::relative(entry.path(), scene_dir);
                            std::string normalised_path = Util::NormaliseFilePathToString(relative_path);
                            GUID scene_id = Util::HashStringInsensitive(normalised_path);

                            bool included = scene_file_paths.contains(scene_id);

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted(relative_path.stem().string().c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", std::to_string(scene_id).c_str());

                            ImGui::TableSetColumnIndex(2);
                            bool checkbox_state = included;
                            std::string checkbox_id = "##Include_" + relative_path.stem().string();

                            if (ImGui::Checkbox(checkbox_id.c_str(), &checkbox_state))
                            {
                                if (checkbox_state)
                                {
                                    scene_file_paths[scene_id] = relative_path;
                                }
                                else
                                {
                                    // Prevent removing the last remaining scene
                                    if (scene_file_paths.size() > 1)
                                        scene_file_paths.erase(scene_id);
                                }

                                // Ensure entry scene remains valid
                                if (!scene_file_paths.contains(entry_scene) && !scene_file_paths.empty())
                                    scene_manager->SetEntryScene(scene_file_paths.begin()->first);
                            }
                            
                            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
                            {
                                ImGui::SetTooltip("Should This Scene be Included in Final Build.");
                            }
                        }

                        ImGui::EndTable();
                    }
                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

    class SceneConfigPanel : public IEditorPanel
    {

    public:

        SceneConfigPanel() = default;
        ~SceneConfigPanel() = default;

        PanelType GetType() const override { return PanelType_SceneConfig; }

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