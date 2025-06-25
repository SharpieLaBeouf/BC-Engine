#include "BC_PCH.h"
#include "PanelBase.h"

namespace BC
{

    namespace Util
    {
        const char *PanelTypeToString(PanelType type)
        {
            switch (type)
            {
                case PanelType_Unknown:              return "Unknown";
                case PanelType_Inspector:            return "Inspector";
                case PanelType_Hierarchy:            return "Hierarchy";
                case PanelType_ContentBrowser:       return "Content Browser";
                case PanelType_AssetRegistry:        return "Asset Registry";
                case PanelType_SceneViewport:        return "Scene";
                case PanelType_Console:              return "Console";
                case PanelType_Profiler:             return "Profiler";
                case PanelType_Statistics:           return "Statistics";
                case PanelType_HumanoidConfig:       return "Humanoid Config";
                case PanelType_AnimatorNodeGraph:    return "Animator";
                case PanelType_SceneManagerConfig:   return "Scene Manager Config";
                case PanelType_SceneConfig:          return "Scene Config";
            }
            return "PanelType_Unknown";
        }

        PanelType PanelTypeFromString(const std::string &str)
        {
            static const std::unordered_map<std::string, PanelType> lookup = {
                { "PanelType_Unknown",      PanelType_Unknown },
                { "Inspector",              PanelType_Inspector },
                { "Hierarchy",              PanelType_Hierarchy },
                { "Content Browser",        PanelType_ContentBrowser },
                { "Asset Registry",         PanelType_AssetRegistry },
                { "Scene",                  PanelType_SceneViewport },
                { "Console",                PanelType_Console },
                { "Profiler",               PanelType_Profiler },
                { "Statistics",             PanelType_Statistics },
                { "Humanoid Config",        PanelType_HumanoidConfig },
                { "Animator",               PanelType_AnimatorNodeGraph },
                { "Scene Manager Config",    PanelType_SceneManagerConfig },
                { "Scene Config",           PanelType_SceneConfig }
            };

            auto it = lookup.find(str);
            return (it != lookup.end()) ? it->second : PanelType_Unknown;
        }

    }

}