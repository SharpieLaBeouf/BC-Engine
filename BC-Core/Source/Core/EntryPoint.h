#pragma once

#include "Application.h"
#include "../Debug/ErrorCodes.h"

namespace BC
{
	int Main_Entry(int argc, char** argv)
	{
		BC_CATCH_BEGIN();
		
		BC::LoggingSystem::Init();

		auto app = CreateApplication({ argc, argv });

		if (!app)
			return BCResult::BC_ERROR_UNKNOWN;

		app->Run();

		delete app;

		BC_CATCH_END_RETURN(BCResult::BC_ERROR_UNKNOWN);

		return BC_SUCCESS;
	}
}

#if defined(NDEBUG) && defined(BC_PLATFORM_WINDOWS)

#include <Windows.h>
#pragma comment(linker, "/subsystem:windows")
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
{
	return BC::Main_Entry(__argc, __argv);
}

#else

#pragma comment(linker, "/subsystem:console")
int main(int argc, char** argv) 
{
	return BC::Main_Entry(argc, argv);
}

#endif