#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "ThreadManager.h"

CoreGlobal Core;

void ThreadMain()
{
	while (true)
	{
		cout << "Hello " << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	for (int32 i = 0; i < 5; ++i) {
		g_threadManager->Launch(ThreadMain);
	}
	g_threadManager->Join();
}