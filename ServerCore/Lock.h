#pragma once
#include "Types.h"

// 기존 mutex의 문제점
// 1. recursive_mutex 지원 X
//	- 별도의 recursive_mutex를 사용할 수 있다.
// 2. read/write lock 구현 X
//	- 모든 쓰레드가 Read만 하고 있는 상황에서는 Lock을 걸 필요가 없다.

/*
 *	Read/Write SpinLock
 */

// 32비트 변수를 사용한다.
// [WWWWWWWW] [WWWWWWWW] [RRRRRRRR] [RRRRRRRR]
// 상위 16비트는 Write Flag를 의미한다.
// Write Flag에는 현재 Lock을 획득중인 쓰레드의 ID를 입력한다.
// 하위 16비트는 Read Flag를 의미한다.
// Read Flag에는 현재 읽고 있는 쓰레드의 개수를 입력한다.

// 동일한 쓰레드가 Write Lock을 잡은 채로 Read Lock을 잡는 것은 허용한다.
// 그러나 Read Lock을 잡은 채로 Write Lock을 잡는 것은 허용하지 않는다.

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
