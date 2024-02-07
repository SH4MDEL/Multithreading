#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PushLock(name);
#endif

	// ������ �����尡 Lock�� �����ϰ� �ִٸ� ������ �����ؾ��Ѵ�.
	const uint32 lockThreadId = (m_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) {
		++m_writeCount;
		return;
	}


	// �ƹ��� ����(Write) �� ����(Read)�ϰ� ���� ���� ��, �����ؼ� �������� ��´�.
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

	// �ϳ��� Read�� ��� �ִٸ� Write�� ���� ���Ѵ�.
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	g_deadLockProfiler->PopLock(name);
#endif

	// ReadLock �� Ǯ�� ���� WriteUnlock �Ұ���
	if ((m_lockFlag.load() & READ_COUNT_MASK) != 0) {
		// �� ReadLock�� �� Ǯ������ �ʾҴµ� WriteLock�� �����Ϸ��� �ϰ� �ִ�.
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

	// ������ �����尡 ����(Write)�ϰ� �ִٸ� ������ ����
	const uint32 lockThreadId = (m_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) {
		m_lockFlag.fetch_add(1);
		return;
	}

	// �ƹ��� ����(Write)�ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���.
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
