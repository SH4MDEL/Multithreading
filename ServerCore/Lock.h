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
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> m_lockFlag = EMPTY_FLAG;
	uint16 m_writeCount = 0;
};


/*
 *	Lock Guard
 */

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : m_lock(lock), m_name(name) { m_lock.ReadLock(name); }
	~ReadLockGuard() { m_lock.ReadUnlock(m_name); }

private:
	Lock& m_lock;
	const char* m_name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : m_lock(lock), m_name(name) { m_lock.WriteLock(name); }
	~WriteLockGuard() { m_lock.WriteUnlock(m_name); }

private:
	Lock& m_lock;
	const char* m_name;
};
