#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

ThreadManager* g_threadManager = nullptr;

// ���� �Ŵ����� ������ �� ����/�Ҹ� ������ ������ �ֱ� ���� Ŭ����
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