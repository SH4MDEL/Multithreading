#pragma once
#include <mutex>
#include "pch.h"

template<typename T>
class SPLockFreeStack
{
private:
	struct Node;
	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{

		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = m_head;
		while (!m_head.compare_exchange_weak(node.ptr->next, node)) {}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = m_head;
		while (true)
		{
			// 참조권 획득
			IncreaseHeadCount(oldHead);
			// 최소한 externalCount가 2보다 크다. 삭제되지 않음
			Node* ptr = oldHead.ptr;

			// 데이터 없음
			if (!ptr) return shared_ptr<T>();

			// 소유권 획득 (ptr->next로 head를 바꿔치기 한 쓰레드가 이김)
			if (m_head.compare_exchange_strong(oldHead, ptr->next)) {
				shared_ptr<T> res;
				res.swap(ptr->data);

				// external : 1 -> 2(+1) ->4(나+1, 남+2)
				// internal : 0

				// 나 말고 또 누가 있는가?
				const int32 countIncrease = oldHead.externalCount - 2;
				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease) delete ptr;

				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1){
				// 참조권은 얻었으나 소유권 획득 실패 -> 뒷수습은 내가 함
				delete ptr;
			}
		}
	}

private:
	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;
			if (m_head.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> m_head;
};


template<typename T>
class LockFreeStack
{
private:
	struct Node
	{
		Node(const T& value) : data(value), next{nullptr}
		{

		}

		T data;
		Node* next;
	};

public:
	// 1) 새 노드를 만들고
	// 2) 새 노드의 next = head
	// 3) head = 새 노드
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = m_head;

		while (!m_head.compare_exchange_weak(node->next, node)) {}
	}

	// 1) head 읽기
	// 2) head->next 읽기
	// 3) head = head->next;
	// 4) data 추출해서 반환
	// 5) 추출한 노드를 삭제
	bool TryPop(T& value)
	{
		++m_popCount;
		Node* oldHead = m_head;
		while (oldHead && !m_head.compare_exchange_weak(oldHead, oldHead->next)) {}

		if (!oldHead) {
			--m_popCount;
			return false;
		}

		value = oldHead->data;
		TryDelete(oldHead);
		return true;
	}

private:
	// 1) 데이터 분리해서 스택에서 빼냄
	// 2) count 체크
	// 3) 나 혼자면 삭제
	void TryDelete(Node* oldHead)
	{
		// 나 외에 누가 있는가?
		if (m_popCount == 1) {
			Node* node = m_pendingList.exchange(nullptr);
			if (!--m_popCount) {
				// 끼어든 쓰레드가 없음 -> 삭제 진행 가능
				// 이제와서 다른 쓰레드가 끼어들어도, 어차피 데이터는 분리해두었다.
				DeleteNodes(node);
			}
			else if (node) {
				// 누가 끼어들었으니 다시 갖다 놓자.
				ChainPendingNodeList(node);
			}

			delete oldHead;
			// 이거 해도 되는거 맞아? 들어왔을때 popCount가 증가할 수도 있잖아.
			// 해도 된다. 왜냐면 이미 이 oldHead는 CAS 연산을 끝냈다.
			// 그리고 CAS를 통해 선택받은 단 하나의 쓰레드만이 이 함수를 실행한다.
			// 다른 쓰레드가 더 이상 이 노드에 접근할 여지가 전혀 없다.
		}
		else {
			// 삭제 안하고 삭제 예약만 하자.
			ChainPendingNode(oldHead);
			--m_popCount;
		}
	}

	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = m_pendingList;
		while (!m_pendingList.compare_exchange_weak(last->next, first)) {}
	}

	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		while (last->next) {
			last = last->next;
		}
		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node*> m_head;
	atomic<uint32> m_popCount = 0;	// pop을 실행중인 쓰레드 개수
	atomic<Node*> m_pendingList; // 삭제 되어야 할 노드들 (첫번째 노드)
};

template<typename T>
class LockStack
{
public:
	LockStack() {}

	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(m_mutex);
		m_stack.push(std::move(value));
		m_cv.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(m_mutex);
		if (m_stack.empty()) return false;

		
		value = std::move(m_stack.top());
		m_stack.pop();
		return true;
	}

	// 꺼내 쓸 데이터가 있을 때까지 기다리는 버전
	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(m_mutex);	// 중간에 lock을 풀어야 하므로 unique_lock
		m_cv.wait(lock, [this] {return m_stack.empty() == false; });	// 락을 잡고 들어가서 조건을 만족하지 않으면 lock 해제
		value = std::move(m_stack.top());
		m_stack.pop();
	}

	// 크게 의미가 없다.
	bool Empty()
	{
		lock_guard<mutex> lock(m_mutex);
		return m_stack.empty();
	}

private:
	stack<T> m_stack;
	mutex m_mutex;
	condition_variable m_cv;
};