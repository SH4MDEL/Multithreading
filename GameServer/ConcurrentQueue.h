#pragma once
#include <mutex>

template<typename T>
class LockQueue
{
public:
	LockQueue() {}

	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(m_mutex);
		m_queue.push(std::move(value));
		m_cv.notify_one();	// 일어나라고 알림
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(m_mutex);
		if (m_queue.empty()) return false;

		value = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	// 꺼내 쓸 데이터가 있을 때까지 기다리는 버전
	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(m_mutex);	// 중간에 lock을 풀어야 하므로 unique_lock
		m_cv.wait(lock, [this] { return m_queue.empty() == false; });	// 락을 잡고 들어가서 조건을 만족하지 않으면 lock 해제
		value = std::move(m_queue.front());
		m_queue.pop();
	}

	// 크게 의미가 없다.
	bool Empty()
	{
		lock_guard<mutex> lock(m_mutex);
		return m_queue.empty();
	}

private:
	queue<T> m_queue;
	mutex m_mutex;
	condition_variable m_cv;
};