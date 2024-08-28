
#pragma once

#include "Application.h"

extern strafe::Application* strafe::CreateApplication();

int main()
{
#ifdef STRAFE_DEBUG
	// Flag _CrtDumpMemoryLeak to be called AFTER program ends.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	auto app = strafe::CreateApplication();

	strafe::Application::ApplicationConfig config;
	app->Init(config);
	app->Run();
	app->Shutdown();

	delete app;
#ifdef STRAFE_DEBUG
	_CrtDumpMemoryLeaks();
#endif
}