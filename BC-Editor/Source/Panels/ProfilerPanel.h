#pragma once

#include "PanelBase.h"

namespace BC
{

    class ProfilerPanel : public IEditorPanel
    {

    public:

        ProfilerPanel() = default;
        ~ProfilerPanel() = default;

        PanelType GetType() const override { return PanelType_Profiler; }

        void OnUpdate() override
        {
        }

        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {
                    if (ImGui::BeginTabBar("ProfilerTabs"))
                    {
                        // --- Frame Profiler Tab ---
                        if (ImGui::BeginTabItem("Frame Profiler"))
                        {
                            static float timer = 1.0f;
                            static bool IsResultsPerFrame = true;
                            static float fps = ImGui::GetIO().Framerate;
                            static std::map<std::string, BC::ProfileResult> results;

                            if (timer > 0.0f && !IsResultsPerFrame)
                                timer -= BC::Time::GetDeltaTime();

                            if (timer <= 0.0f || IsResultsPerFrame)
                            {
                                timer = 1.0f;
                                results = BC::Profiler::Get().GetResults();
                                fps = ImGui::GetIO().Framerate;
                            }

                            ImGui::Text("FrameRate: %.2f", fps);

                            ImGui::Dummy({0.0f, 2.5f});
                            ImGui::Separator();
                            ImGui::Dummy({0.0f, 2.5f});

                            for (auto& result : results)
                            {
                                ImGui::Text("%s: %.2f", result.second.Name.c_str(), result.second.Time);
                            }

                            ImGui::Dummy({0.0f, 2.5f});
                            ImGui::Separator();
                            ImGui::Dummy({0.0f, 2.5f});

                            ImGui::Checkbox("Toggle Results Per Frame/Per Second", &IsResultsPerFrame);

                            ImGui::EndTabItem();
                        }

                        // --- Job System Tab ---
                        if (ImGui::BeginTabItem("Job System"))
                        {
                            static std::unordered_map<std::string, ImU32> job_colour_map;

                            auto get_job_colour = [](const std::string& name) -> ImU32 {
                                auto it = job_colour_map.find(name);
                                if (it != job_colour_map.end()) return it->second;

                                std::hash<std::string> hasher;
                                size_t hash = hasher(name);
                                ImU8 r = 100 + (hash & 0x5F);
                                ImU8 g = 100 + ((hash >> 8) & 0x5F);
                                ImU8 b = 100 + ((hash >> 16) & 0x5F);

                                ImU32 color = IM_COL32(r, g, b, 220);
                                job_colour_map[name] = color;
                                return color;
                            };

                            static auto profile = Application::GetJobSystem()->GetProfileResults(1);
                            static bool profiling_is_paused = false;
                            if (ImGui::Button(profiling_is_paused ? "Resume" : "Pause"))
                                profiling_is_paused = !profiling_is_paused;

                            if (Input::GetKey(Key::F11))
                                profiling_is_paused = true;

                            if (!profiling_is_paused)
                                profile = Application::GetJobSystem()->GetProfileResults(1);

                            float total_width = ImGui::GetContentRegionAvail().x;
                            float total_height = ImGui::GetContentRegionAvail().y;

                            float header_width = 100.0f;
                            int num_threads = static_cast<int>(profile.thread_events.size());
                            float row_height = total_height / static_cast<float>(num_threads);
                            float graph_width = total_width - header_width;

                            ImGui::BeginChild("JobProfileGraph", ImVec2(total_width, total_height), true);
                            ImDrawList* job_draw_list = ImGui::GetWindowDrawList();
                            ImVec2 graph_pos = ImGui::GetCursorScreenPos();

                            job_draw_list->AddRect(graph_pos, ImVec2(graph_pos.x + total_width, graph_pos.y + total_height), IM_COL32(180, 180, 180, 255));

                            for (int t = 0; t < num_threads; t++)
                            {
                                float rowY = graph_pos.y + row_height * t;
                                if (t % 2 == 0)
                                    job_draw_list->AddRectFilled({graph_pos.x, rowY}, {graph_pos.x + total_width, rowY + row_height}, IM_COL32(30, 30, 30, 100));

                                std::string label = "Core " + std::to_string(t + 3);
                                ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
                                job_draw_list->AddText({graph_pos.x + 8.0f, rowY + (row_height - text_size.y) * 0.5f}, IM_COL32(255, 215, 0, 255), label.c_str());

                                job_draw_list->AddLine({graph_pos.x, rowY}, {graph_pos.x + total_width, rowY}, IM_COL32(60, 60, 60, 255));

                                for (const auto& event : profile.thread_events[t])
                                {
                                    float startX = graph_pos.x + header_width + (event.StartTime / profile.frame_duration) * graph_width;
                                    float endX = graph_pos.x + header_width + (event.EndTime / profile.frame_duration) * graph_width;
                                    ImVec2 job_min(startX, rowY + 4.0f);
                                    ImVec2 job_max(endX, rowY + row_height - 4.0f);

                                    ImU32 fill_colour = get_job_colour(event.Name);

                                    ImU32 outline_colour =
                                        (event.Priority == JobPriority::High)   ? IM_COL32(255, 50, 50, 255) :
                                        (event.Priority == JobPriority::Medium) ? IM_COL32(255, 165, 0, 255) :
                                        (event.Priority == JobPriority::Low)    ? IM_COL32(0, 200, 0, 255) :
                                                                                IM_COL32(200, 200, 200, 255);

                                    job_draw_list->AddRectFilled(job_min, job_max, fill_colour, 4.0f);
                                    job_draw_list->AddRect(job_min, job_max, outline_colour, 4.0f, 0, 1.0f);

                                    std::string label_text = event.Name + " (" + std::to_string(event.EndTime - event.StartTime) + "ms)";
                                    ImVec2 label_size = ImGui::CalcTextSize(label_text.c_str());

                                    ImVec2 text_pos = {
                                        std::max(job_min.x + 2.0f, job_min.x + ((job_max.x - job_min.x - label_size.x) * 0.5f)),
                                        job_min.y + ((job_max.y - job_min.y - label_size.y) * 0.5f)
                                    };

                                    job_draw_list->PushClipRect(job_min, job_max, true);
                                    job_draw_list->AddText(text_pos, IM_COL32(0, 0, 0, 255), label_text.c_str());
                                    job_draw_list->PopClipRect();

                                    ImVec2 mouse_pos = ImGui::GetIO().MousePos;
                                    if (mouse_pos.x >= job_min.x && mouse_pos.x <= job_max.x &&
                                        mouse_pos.y >= job_min.y && mouse_pos.y <= job_max.y)
                                    {
                                        ImGui::BeginTooltip();
                                        ImGui::Text("Job: %s", event.Name.c_str());
                                        ImGui::Text("Start: %.3f ms", event.StartTime);
                                        ImGui::Text("End: %.3f ms", event.EndTime);
                                        ImGui::Text("Duration: %.3f ms", event.EndTime - event.StartTime);
                                        const char* priorityStr =
                                            (event.Priority == JobPriority::High) ? "High" :
                                            (event.Priority == JobPriority::Medium) ? "Medium" :
                                            (event.Priority == JobPriority::Low) ? "Low" : "Unknown";
                                        ImGui::Text("Priority: %s", priorityStr);
                                        ImGui::EndTooltip();
                                    }
                                }
                            }

                            job_draw_list->AddLine({graph_pos.x + header_width, graph_pos.y}, {graph_pos.x + header_width, graph_pos.y + total_height}, IM_COL32(255, 255, 255, 80));
                            job_draw_list->AddLine({graph_pos.x + total_width, graph_pos.y}, {graph_pos.x + total_width, graph_pos.y + total_height}, IM_COL32(255, 255, 255, 80));

                            ImGui::EndChild();
                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                }
                ImGui::End();
            }
        }


    private:

        friend class EditorLayer;

    };

}