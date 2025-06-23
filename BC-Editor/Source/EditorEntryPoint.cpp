#include "BC_PCH.h"

#include "BC-Editor.h"

#include "Core/EntryPoint.h"

namespace BC
{
    class BCEditorApplication : public Application 
    {

    public:

        BCEditorApplication(const BCApplicationSpecification& specification) : Application(specification) 
        {
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

        return new BC::BCEditorApplication(spec);
    }
}