#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

mutex m;
LockQueue<int32> q;
LockFreeStack<int32> s;

void Push()
{
	while (true)
	{
		int32 value = rand() % 100;
		s.Push(value);

		this_thread::sleep_for(10ms);
	}
}

void Pop()
{
	while (true)
	{
		int32 data;
		if (s.TryPop(data)) {
			//cout << data << endl;
		}
	}
}

int main()
{
	thread t1(Push);
	thread t2(Push);
	thread t3(Pop);
	thread t4(Pop);
	thread t5(Pop);

	t1.join(); t2.join();
	t3.join(); t4.join();
	t5.join();

}