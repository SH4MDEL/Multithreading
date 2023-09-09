#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

mutex m;
queue<int32> q;
condition_variable cv;

class SpinLock
{
public:
	void lock() {
		bool expected = false;
		const bool desired = true;

		while (m_locked.compare_exchange_strong(expected, desired) == false) 
		{
			expected = false;

			this_thread::sleep_for(std::chrono::milliseconds(100));
			//this_thread::yield();
		}
	}
	void unlock() {
		m_locked.store(false);
	}

private:
	atomic_bool m_locked = false;
};

void Producer()
{
	while (true)
	{
		// 1) Lock을 잡음
		// 2) 공유 변수 값을 수정
		// 3) Lock을 품
		// 4) 조건 변수를 통해 다른 쓰레드에게 통지
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait 중인 쓰레드가 있으면 딱 하나를 깨운다.
	}
}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() {
			return !q.empty();
			});
		// 1) Lock을 잡으려고 시도하고
		// 2) 조건 확인
		// - 만족할 경우 빠져 나와서 코드 진행
		// - 만족하지 않을 경우 Lock을 풀어주고 대기 상태

		// 그런데 notify_one을 했으면 항상 조건식을 만족하는 것 아닐까?
		// Squrious Wakeup
		// notify_one 할 때 lock을 잡고 있는 것이 아니기 때문에 다시 확인한다.

		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
		}
	}
}

int64 result;

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; ++i) {
		sum += i;
	}
	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{
	// 동기 실행 (synchronous) 실행
	int64 sum = Calculate();
	cout << sum << endl;

	// std::future
	{
		// 1) deferred -> lazy evaluation : 지연해서 실행하세요
		// 2) async -> 별도의 쓰레드를 만들어서 실행하세요
		// 3) deferred | async -> 둘 중 알아서 골라주세요

		// 언젠가 미래에 결과물을 뱉어줄거야
		std::future<int64> future = std::async(std::launch::async, Calculate);

		// TODO
		std::future_status status = future.wait_for(1ms);
		if (status == future_status::ready) {

		}

		int64 sum = future.get();

		class Knight
		{
		public:
			int64 GetHp() { return 100; }
		};
		Knight knight;
		//std::future<int64> future = std::async(std::launch::async, &Knight::GetHp, knight);

	}

	// std::promise
	{
		// 미래(std::future)에 결과물을 반환해줄거라 약속
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}

	// std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		thread t(TaskWorker, move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}
}