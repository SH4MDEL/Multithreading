#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager()
{
	// ���� ������ �ʱ�ȭ
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	// ���� �����忡�� �����ϴ� ������ ��Ȳ�� �����ϰ��� Locking
	LockGuard guard(m_lock);

	m_threads.push_back(thread([=]() {
		InitTLS();
		callback();
		DestroyTLS();
	}));
}

void ThreadManager::Join()
{
	for (auto& thread : m_threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	m_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
