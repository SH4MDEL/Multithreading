#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"

ThreadManager* g_threadManager = nullptr;
DeadLockProfiler* g_deadLockProfiler = nullptr;

// 여러 매니저가 존재할 때 생성/소멸 시점을 관리해 주기 위한 클래스
class CoreGlobal
{
public:
	CoreGlobal()
	{
		g_threadManager = new ThreadManager();
		g_deadLockProfiler = new DeadLockProfiler();
	}
	~CoreGlobal()
	{
		delete g_threadManager;
		delete g_deadLockProfiler;
	}
} h_coreGlobal;