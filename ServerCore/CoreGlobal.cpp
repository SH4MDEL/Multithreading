#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

ThreadManager* g_threadManager = nullptr;

// 여러 매니저가 존재할 때 생성/소멸 시점을 관리해 주기 위한 클래스
class CoreGlobal
{
public:
	CoreGlobal()
	{
		g_threadManager = new ThreadManager();
	}
	~CoreGlobal()
	{
		delete g_threadManager;
	}
} h_coreGlobal;