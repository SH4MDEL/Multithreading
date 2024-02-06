#pragma once

// 전역으로 사용하는 변수를 관리한다.
extern class ThreadManager* g_threadManager;


// 여러 매니저가 존재할 때 생성/소멸 시점을 관리해 주기 위한 클래스
class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();
};

