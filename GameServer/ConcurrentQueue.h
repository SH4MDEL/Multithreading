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
		m_cv.notify_one();	// �Ͼ��� �˸�
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(m_mutex);
		if (m_queue.empty()) return false;

		value = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	// ���� �� �����Ͱ� ���� ������ ��ٸ��� ����
	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(m_mutex);	// �߰��� lock�� Ǯ��� �ϹǷ� unique_lock
		m_cv.wait(lock, [this] { return m_queue.empty() == false; });	// ���� ��� ���� ������ �������� ������ lock ����
		value = std::move(m_queue.front());
		m_queue.pop();
	}

	// ũ�� �ǹ̰� ����.
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