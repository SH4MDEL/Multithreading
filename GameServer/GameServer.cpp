#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock() {
		// CAS (Compare And Swap)

		//if (m_locked == expected) {
		//	m_locked = desired;
		//	return true;
		//}
		//else {
		//	expected = m_locked;
		//	return false;
		//}

		bool expected = false;
		const bool desired = true;

		while (m_locked.compare_exchange_strong(expected, desired) == false) 
		{
			expected = false;
		}
	}
	void unlock() {
		m_locked.store(false);
	}

private:
	atomic_bool m_locked = false;
};

mutex m;
int32 sum;
SpinLock spinLock;

void Add()
{
	for (int32 i = 0; i < 100000; ++i) {
		lock_guard<SpinLock> g(spinLock);
		++sum;
	}
}

void Sub()
{
	for (int32 i = 0; i < 100000; ++i) {
		lock_guard<SpinLock> g(spinLock);
		--sum;
	}
}

int main()
{
	thread t1(Add);
	thread t2(Sub);
	t1.join(); t2.join();

	cout << sum << endl;
}