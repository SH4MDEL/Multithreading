#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"

ThreadManager* g_threadManager = nullptr;
DeadLockProfiler* g_deadLockProfiler = nullptr;

// ���� �Ŵ����� ������ �� ����/�Ҹ� ������ ������ �ֱ� ���� Ŭ����
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