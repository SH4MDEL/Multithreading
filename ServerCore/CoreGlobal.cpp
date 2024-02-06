#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

ThreadManager* g_threadManager = nullptr;

CoreGlobal::CoreGlobal()
{
	g_threadManager = new ThreadManager();
}

CoreGlobal::~CoreGlobal()
{
	delete g_threadManager;
}
