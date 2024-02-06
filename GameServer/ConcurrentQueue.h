#pragma once
#include <mutex>

template<typename T>
class LockFreeQueue
{
private:
	struct Node;
	struct CountedNodePtr
	{
		int32 externalCount;	// 참조권
		Node* ptr = nullptr
	};

	struct NodeCounter
	{
		uint32 internalCount : 30;			// 참조권 반환 관련
		uint32 externalCountRemaining : 2;	// Push & Pop 다중 참조권 관련
	};

	struct Node {
		Node()
		{
			NodeCounter newCount;
			newCount.internalCount = 0;
			newCount.externalCountRemaining = 2;
			count.store(newCount);

			next.ptr = nullptr;
			next.externalCount = 0;
		}

		void ReleaseRef()
		{
			NodeCounter oldCounter = count.load();

			while (true) {
				NodeCounter newCounter = oldCounter;
				newCounter.internalCount--;

				// 끼어들 수 있음
			}
		}

		atomic<T*> data;
		atomic<NodeCounter> count;
		CountedNodePtr next;
	};

public:
	LockFreeQueue() : m_head(new Node), m_tail(m_head) {}

	LockFreeQueue(const LockQueue&) = delete;
	LockFreeQueue& operator=(const LockQueue&) = delete;

	void Push(const T& value)
	{
		unique_ptr<T> newData = make_unique<T>(value);

		CountedNodePtr dummy;
		dummy.ptr = new Node;
		dummy.externalCount = 1;

		CountedNodePtr oldTail = m_tail.load();

		while (true)
		{
			// 참조권 획득 (externalCount를 현시점 기준 +1 한 애가 이김)
			IncreaseExternalCount(m_tail, oldTail);

			// 소유권 획득 (data를 먼저 교환한 쓰레드가 이김)
			T* oldData = nullptr;
			if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get())) {
				oldTail.ptr->next = dummy;
				oldTail = m_tail.exchange(dummy);
				FreeExternalCount(oldTail);
				newData.release();	// 데이터에 대한 unique_ptr의 소유권 포기
				break;
			}

			// 소유권 경쟁 패배
			oldTail.ptr->ReleaseRef();
		}
	}

	shared_ptr<T> TryPop()
	{
		//Node* oldHead = PopHead();
		//if (!oldHead) return shared_ptr<T>();

		//shared_ptr<T> res(oldHead->data);
		//delete oldHead;
		//return res;
	}
private:
	static void IncreaseExternalCount(atomic<CountedNodePtr>& counter, CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (counter.compare_exchange_strong(oldCounter, newCounter)) {
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

	static void FreeExternalCount(CountedNodePtr& oldNodePtr)
	{
		Node* ptr = oldNodePtr.ptr;
		count int32 countIncrease = oldNodePtr.externalCount - 2;
		NodeCounter oldCounter = ptr->count.load();

		while (true)
		{
			NodeCounter newCounter = oldCounter;
			newCounter.externalCountRemaining--;
			newCounter.internalCount += countIncrease;

			if (ptr->count.compare_exchange_strong(oldCounter.newCounter)) {
				if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0) {
					delete ptr;
				}

				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> m_head;
	atomic<CountedNodePtr> m_tail;
};

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