#include "BC_PCH.h"
#include "Project.h"

#include "Util/Hash.h"
#include "Util/FileUtil.h"

#include <yaml-cpp/yaml.h>

namespace BC
{

    static void CreateProjectDirectories(const std::filesystem::path& project_directory)
    {
        std::filesystem::create_directories(project_directory / "Assets");
        std::filesystem::create_directories(project_directory / "Scenes");
        std::filesystem::create_directories(project_directory / "Scripts");

        std::filesystem::create_directories(project_directory / "Scripts/Bin");
        std::filesystem::create_directories(project_directory / "Scripts/Generated");
        std::filesystem::create_directories(project_directory / "Scripts/ScriptCoreAPI");
    }

    std::unique_ptr<Project> Project::NewProject(const std::filesystem::path& project_file_path, bool editor)
    {
        BC_THROW(Project::ValidProjectFilePath(project_file_path), "Project::LoadProject: Invalid Project File Path Provided.");

        CreateProjectDirectories(project_file_path.parent_path());
        
        std::unique_ptr<Project> project = std::make_unique<Project>();
        project->m_Name = project_file_path.stem().string();
        project->m_Directory = project_file_path.parent_path();

        // 1. Asset Manager
        if (editor)
            project->m_AssetManager = std::make_unique<EditorAssetManager>();
        else
            project->m_AssetManager = std::make_unique<RuntimeAssetManager>();

        // 2. Default Scene
        auto default_scene = Scene::CreateDefaultScene("Untitled Scene.scene");

        project->m_SceneManager->AddSceneTemplate(default_scene);
        project->m_SceneManager->m_EntryScene = default_scene->m_SceneID;
        project->m_SceneManager->m_ActiveScene = default_scene->m_SceneID;

        return std::move(project);
    }

    std::unique_ptr<Project> Project::LoadProject(const std::filesystem::path& project_file_path, bool editor)
    {
        BC_THROW(Project::ValidProjectFilePath(project_file_path), "Project::LoadProject: Invalid Project File Path Provided.");

        CreateProjectDirectories(project_file_path.parent_path());

        std::unique_ptr<Project> project = std::make_unique<Project>();
        project->m_Name = project_file_path.stem().string();
        project->m_Directory = project_file_path.parent_path();

        if (editor)
            project->m_AssetManager = std::make_unique<EditorAssetManager>();
        else
            project->m_AssetManager = std::make_unique<RuntimeAssetManager>();

        BC_THROW(project->Deserialise(project_file_path), "Project::LoadProject: Could Not Load Project.");

        return std::move(project);
    }

    Project::Project()
    {
        m_SceneManager->OnStart();
    }

    Project::~Project()
    {
        m_SceneManager->OnStop();
    }

    void Project::SaveProject()
    {
        m_SceneManager->SaveAllScenes();
        
        Serialise();
    }

    void Project::ReloadProject()
    {
        Deserialise(m_Directory / (m_Name + ".project"));
    }

    void Project::Serialise() const
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        {
            out << YAML::Key << "Project Name" << YAML::Value << m_Name;
            m_SceneManager->Serialise(out);
        }
        out << YAML::EndMap;

        std::filesystem::create_directories(m_Directory);

        std::ofstream fout(std::filesystem::path(m_Directory / (m_Name + ".project")));
        BC_ASSERT(fout.is_open(), "Project::SaveProject: Could Not Open File.");
        fout << out.c_str();
    }

    bool Project::Deserialise(const std::filesystem::path& project_file_path)
    {
        if (!ValidProjectFilePath(project_file_path))
            return false;

        YAML::Node data = YAML::LoadFile(project_file_path.string()); // If Exception - Will Be Caught by Main Try/Catch

        if (data["Project Name"])
            m_Name = data["Project Name"].as<std::string>();

        m_SceneManager.reset();
        m_SceneManager = std::make_unique<SceneManager>();
        m_SceneManager->Deserialise(data["Scene Manager"], project_file_path.parent_path());
        m_SceneManager->LoadScene(m_SceneManager->m_EntryScene, false, project_file_path.parent_path());

        return true;
    }

    bool Project::ValidProjectFilePath(const std::filesystem::path& project_file_path)
    {
        if (project_file_path.extension() != ".project")
        {
            BC_CORE_ERROR("Project::ValidProjectFilePath: Incorrect Extension - Expected '.project', but got '{}'.", project_file_path.extension().string());
            return false;
        }

        if (!project_file_path.has_parent_path())
        {
            BC_CORE_ERROR("Project::ValidProjectFilePath: Does Not Contain Parent Path.");
            return false;
        }
        
        return true;
    }

}