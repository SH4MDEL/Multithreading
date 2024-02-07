#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 Lock을 소유하고 있다면 무조건 성공해야한다.
	const uint32 lockThreadId = (m_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) {
		++m_writeCount;
		return;
	}


	// 아무도 소유(Write) 및 공유(Read)하고 있지 않을 때, 경합해서 소유권을 얻는다.
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount) {
			uint32 expected = EMPTY_FLAG;
			if (m_lockFlag.compare_exchange_strong(OUT expected, desired)) {
				++m_writeCount;
				return;
			}
		}

		if (::GetTickCount64() - beginTick > ACQUIRE_TIMEOUT_TICK) {
			CRASH("LOCK_TIMEOUT");
		}

		this_thread::yield();

	}

	// 하나라도 Read를 잡고 있다면 Write를 잡지 못한다.
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PopLock(name);
#endif

	// ReadLock 다 풀기 전엔 WriteUnlock 불가능
	if ((m_lockFlag.load() & READ_COUNT_MASK) != 0) {
		// 내 ReadLock이 다 풀리지도 않았는데 WriteLock을 해제하려고 하고 있다.
		CRASH("INVALID_UNLOCK_MASK");
	}
	const int32 lockCount = --m_writeCount;
	if (lockCount == 0) {
		m_lockFlag.store(EMPTY_FLAG);
	}
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유(Write)하고 있다면 무조건 성공
	const uint32 lockThreadId = (m_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) {
		m_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유(Write)하고 있지 않을 때 경합해서 공유 카운트를 올린다.
	const int64 beginTick = ::GetTickCount64();
	while (true) 
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount) {
			uint32 expected = (m_lockFlag.load() & READ_COUNT_MASK);
			if (m_lockFlag.compare_exchange_strong(OUT expected, expected + 1)) {
				return;
			}
		}

		if (::GetTickCount64() - beginTick > ACQUIRE_TIMEOUT_TICK) {
			CRASH("LOCK_TIMEOUT");
		}

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PopLock(name);
#endif

	if ((m_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0) {
		CRASH("MULTIPLE_UNLOCK");
	}
}
