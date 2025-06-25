#include "BC_PCH.h"

#include "BC-Editor.h"

#include "Core/EntryPoint.h"

#ifndef BC_EDITOR
#define BC_EDITOR
#endif

namespace BC
{
    static bool s_NewProjectCreated = false;

    class BCEditorApplication : public Application 
    {

    public:

        BCEditorApplication(BCApplicationSpecification& specification) : Application(specification) 
        {
            if (s_NewProjectCreated)
                GetProject()->SaveProject(); // Save Newly Created Project After Application::m_Project has been set

            PushLayer(new EditorLayer());
        }

        ~BCEditorApplication() = default;
        
    };

    Application* CreateApplication(BC::ApplicationCommandLineArgs args) 
    {
        BCApplicationSpecification spec;
        spec.Name = "BC Engine - Editor";
        spec.WorkingDirectory = "";
        spec.CommandLineArgs = args;
        
        std::filesystem::path project_path;
        for (int i = 0; i < spec.CommandLineArgs.Count; ++i)
        {
            std::string arg = spec.CommandLineArgs.Args[i];

            if (arg == "--project" && i + 1 < spec.CommandLineArgs.Count)
            {
                project_path = spec.CommandLineArgs.Args[i + 1];
                ++i;
            }
            else
            {
                BC_APP_TRACE("BCEditorApplication: Unknown Command Line Argument - {}.", arg);
            }
        }

        if (project_path.empty())
        {
            spec.EntryProject = Project::NewProject("Untitled Project/Untitled Project.project", true);
            s_NewProjectCreated = true;
        }
        else
        {
            spec.EntryProject = Project::LoadProject(project_path, true);
        }

        return new BC::BCEditorApplication(spec);
    }
}