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
		// 1) Lock�� ����
		// 2) ���� ���� ���� ����
		// 3) Lock�� ǰ
		// 4) ���� ������ ���� �ٸ� �����忡�� ����
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait ���� �����尡 ������ �� �ϳ��� �����.
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
		// 1) Lock�� �������� �õ��ϰ�
		// 2) ���� Ȯ��
		// - ������ ��� ���� ���ͼ� �ڵ� ����
		// - �������� ���� ��� Lock�� Ǯ���ְ� ��� ����

		// �׷��� notify_one�� ������ �׻� ���ǽ��� �����ϴ� �� �ƴұ�?
		// Squrious Wakeup
		// notify_one �� �� lock�� ��� �ִ� ���� �ƴϱ� ������ �ٽ� Ȯ���Ѵ�.

		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
		}
	}
}

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

atomic<bool> ready;
int32 value;


void Push()
{
	while (true)
	{
		int32 value = rand() % 100;
		q.push(value);
	}
}

void Pop()
{
	while (true)
	{
		if (q.empty()) continue;
		
		int32 data = q.front();
		q.pop();
		cout << data << endl;
	}
}

int main()
{
	thread t1(Push);
	thread t2(Pop);

	t1.join(); t2.join();

	//ready.store(false);
	//value = 0;
	//thread t1(Producer);
	//thread t2(Consumer);
	//t1.join(); t2.join();

	//// ���� ���� (synchronous) ����
	//{
	//	int64 sum = Calculate();
	//	cout << sum << endl;
	//}

	//// std::future
	//{
	//	// 1) deferred -> lazy evaluation : �����ؼ� �����ϼ���
	//	// 2) async -> ������ �����带 ���� �����ϼ���
	//	// 3) deferred | async -> �� �� �˾Ƽ� ����ּ���

	//	// ������ �̷��� ������� ����ٰž�
	//	std::future<int64> future = std::async(std::launch::async, Calculate);

	//	// TODO
	//	std::future_status status = future.wait_for(1ms);
	//	if (status == future_status::ready) {

	//	}

	//	int64 sum = future.get();

	//	class Knight
	//	{
	//	public:
	//		int64 GetHp() { return 100; }
	//	};
	//	Knight knight;
	//	//std::future<int64> future = std::async(std::launch::async, &Knight::GetHp, knight);

	//}

	//// std::promise
	//{
	//	// �̷�(std::future)�� ������� ��ȯ���ٰŶ� ���
	//	std::promise<string> promise;
	//	std::future<string> future = promise.get_future();

	//	thread t(PromiseWorker, move(promise));

	//	string message = future.get();
	//	cout << message << endl;

	//	t.join();
	//}

	//// std::packaged_task
	//{
	//	std::packaged_task<int64(void)> task(Calculate);
	//	std::future<int64> future = task.get_future();

	//	thread t(TaskWorker, move(task));

	//	int64 sum = future.get();
	//	cout << sum << endl;

	//	t.join();
	//}
}