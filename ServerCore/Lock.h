#pragma once
#include "Types.h"

// ���� mutex�� ������
// 1. recursive_mutex ���� X
//	- ������ recursive_mutex�� ����� �� �ִ�.
// 2. read/write lock ���� X
//	- ��� �����尡 Read�� �ϰ� �ִ� ��Ȳ������ Lock�� �� �ʿ䰡 ����.

/*
 *	Read/Write SpinLock
 */

// 32��Ʈ ������ ����Ѵ�.
// [WWWWWWWW] [WWWWWWWW] [RRRRRRRR] [RRRRRRRR]
// ���� 16��Ʈ�� Write Flag�� �ǹ��Ѵ�.
// Write Flag���� ���� Lock�� ȹ������ �������� ID�� �Է��Ѵ�.
// ���� 16��Ʈ�� Read Flag�� �ǹ��Ѵ�.
// Read Flag���� ���� �а� �ִ� �������� ������ �Է��Ѵ�.

// ������ �����尡 Write Lock�� ���� ä�� Read Lock�� ��� ���� ����Ѵ�.
// �׷��� Read Lock�� ���� ä�� Write Lock�� ��� ���� ������� �ʴ´�.

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};
public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	Atomic<uint32> m_lockFlag;
	uint16 m_writeCount = 0;
};


/*
 *	Lock Guard
 */

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock) : m_lock(lock) { m_lock.ReadLock(); }
	~ReadLockGuard() { m_lock.ReadUnlock(); }

private:
	Lock& m_lock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock) : m_lock(lock) { m_lock.WriteLock(); }
	~WriteLockGuard() { m_lock.WriteUnlock(); }

private:
	Lock& m_lock;
};
