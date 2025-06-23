#pragma once

#include "PanelBase.h"

namespace BC
{

    class ConsolePanel : public IEditorPanel
    {

    public:

        ConsolePanel() = default;
        ~ConsolePanel() = default;

        PanelType GetType() const override { return PanelType_Console; }

        void OnUpdate() override { }
        void OnRenderGUI() override
        {
            if (m_Active)
            {
                if (ImGui::Begin(Util::PanelTypeToString(GetType()), &m_Active))
                {
                    // --- Filtering Section ---
                    // Text filter for arbitrary substring search.
                    static ImGuiTextFilter text_filter;
                    text_filter.Draw("Text Filter");

                    ImGui::Separator();

                    // Logger filters: Toggle display for L_CORE and L_APP logs.
                    static bool show_core = true, show_app = true, show_scripting = true;
                    ImGui::Checkbox("Engine", &show_core);
                    ImGui::SameLine();
                    ImGui::Checkbox("Editor", &show_app);
                    ImGui::SameLine();
                    ImGui::Checkbox("Scripting", &show_scripting);

                    ImGui::Separator();

                    // Log level filters using checkboxes.
                    static bool show_info = true, show_warn = true, show_error = true, show_critical = true, show_trace = false;
                    ImGui::Checkbox("Info", &show_info);
                    ImGui::SameLine();
                    ImGui::Checkbox("Warn", &show_warn);
                    ImGui::SameLine();
                    ImGui::Checkbox("Error", &show_error);
                    ImGui::SameLine();
                    ImGui::Checkbox("Critical", &show_critical);
                    ImGui::SameLine();
                    ImGui::Checkbox("Trace", &show_trace);

                    ImGui::Separator();

                    // Clear logs button
                    if (ImGui::Button("Clear"))
                        CustomLoggingSink::GetInstance().Clear();

                    // --- Scrolling Region for Logs ---
                    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

                    const auto& logs = CustomLoggingSink::GetInstance().GetLogs();
                    for (const auto& entry : logs)
                    {
                        // Filter by logger name.
                        bool logger_pass = (entry.logger_name == "BC_CORE" && show_core) || (entry.logger_name == "BC_APP" && show_app) || (entry.logger_name == "BC_SCRIPTS" && show_scripting);
                        if (!logger_pass)
                            continue;

                        // Filter by log level.
                        bool level_pass = false;
                        switch (entry.level) 
                        {
                            case spdlog::level::trace:    level_pass = show_trace; break;
                            case spdlog::level::info:     level_pass = show_info; break;
                            case spdlog::level::warn:     level_pass = show_warn; break;
                            case spdlog::level::err:      level_pass = show_error; break;
                            case spdlog::level::critical: level_pass = show_critical; break;
                            default:                    level_pass = true; break;
                        }
                        if (!level_pass)
                            continue;

                        // Apply text filter.
                        if (!text_filter.PassFilter(entry.text.c_str()))
                            continue;

                        // Set text color based on log level.
                        ImVec4 colour;
                        switch (entry.level) 
                        {
                            case spdlog::level::info:
                            {
                                colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                                break;
                            }
                            case spdlog::level::warn:
                            {
                                colour = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                                break;
                            }
                            case spdlog::level::err:
                            {
                                colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                                break;
                            }
                            
                            case spdlog::level::trace:
                            case spdlog::level::critical:
                            default:
                            {
                                colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                                break;
                            }
                        }

                        // Render log line with proper styling.
                        if (entry.level == spdlog::level::critical) 
                        {
                            // For critical logs, also change background.
                            ImGui::PushStyleColor(ImGuiCol_Text, colour);
                            // Push a semi-transparent red background.
                            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
                            ImGui::TextUnformatted(entry.text.c_str());
                            ImGui::PopStyleColor(2);
                        }
                        else 
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, colour);
                            ImGui::TextUnformatted(entry.text.c_str());
                            ImGui::PopStyleColor();
                        }
                    }

                    // Auto-scroll to the bottom if already at the bottom.
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);

                    ImGui::EndChild();
                }
                ImGui::End();
            }
        }

    private:

        friend class EditorLayer;

    };

}