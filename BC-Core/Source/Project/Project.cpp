#include "BC_PCH.h"
#include "Project.h"

namespace BC
{
    Project::Project()
    {
        m_SceneManager->OnStart();
    }

    Project::~Project()
    {
        m_SceneManager->OnStop();
    }
}