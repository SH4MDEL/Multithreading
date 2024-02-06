#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "ThreadManager.h"

class TestLock
{
	USE_LOCK;

public:
	int32 TestRead()
	{
		READ_LOCK;

		if (m_queue.empty()) {
			return -1;
		}
		return m_queue.front();
	}

	void TestPush()
	{
		WRITE_LOCK;

		m_queue.push(rand() % 100);
	}

	void TestPop()
	{
		WRITE_LOCK;

		while (true) {}

		if (!m_queue.empty()) {
			m_queue.pop();
		}
	}

private:
	queue<int32> m_queue;
};

TestLock tl;

void ThreadWrite()
{
	while (true)
	{
		tl.TestPush();
		this_thread::sleep_for(1ms);
		tl.TestPop();
	}
}

void ThreadRead()
{
	while (true)
	{
		int32 value = tl.TestRead();
		cout << value << endl;
		this_thread::sleep_for(1ms);
	}
}

int main()
{
	for (int32 i = 0; i < 2; ++i) {
		g_threadManager->Launch(ThreadWrite);
	}
	for (int32 i = 0; i < 5; ++i) {
		g_threadManager->Launch(ThreadRead);
	}
	g_threadManager->Join();
}