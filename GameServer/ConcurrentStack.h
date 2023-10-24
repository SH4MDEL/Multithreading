#pragma once
#include <mutex>

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
	// 1) �� ��带 �����
	// 2) �� ����� next = head
	// 3) head = �� ���
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = m_head;

		while (!m_head.compare_exchange_weak(node->next, node)) {}
	}

	// 1) head �б�
	// 2) head->next �б�
	// 3) head = head->next;
	// 4) data �����ؼ� ��ȯ
	// 5) ������ ��带 ����
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
	// 1) ������ �и��ؼ� ���ÿ��� ����
	// 2) count üũ
	// 3) �� ȥ�ڸ� ����
	void TryDelete(Node* oldHead)
	{
		// �� �ܿ� ���� �ִ°�?
		if (m_popCount == 1) {
			Node* node = m_pendingList.exchange(nullptr);
			if (!--m_popCount) {
				// ����� �����尡 ���� -> ���� ���� ����
				// �����ͼ� �ٸ� �����尡 �����, ������ �����ʹ� �и��صξ���.
				DeleteNodes(node);
			}
			else if (node) {
				// ���� ���������� �ٽ� ���� ����.
				ChainPendingNodeList(node);
			}

			delete oldHead;
			// �̰� �ص� �Ǵ°� �¾�? �������� popCount�� ������ ���� ���ݾ�.
			// �ص� �ȴ�. �ֳĸ� �̹� �� oldHead�� CAS ������ ���´�.
			// �׸��� CAS�� ���� ���ù��� �� �ϳ��� �����常�� �� �Լ��� �����Ѵ�.
			// �ٸ� �����尡 �� �̻� �� ��忡 ������ ������ ���� ����.
		}
		else {
			// ���� ���ϰ� ���� ���ุ ����.
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
	atomic<uint32> m_popCount = 0;	// pop�� �������� ������ ����
	atomic<Node*> m_pendingList; // ���� �Ǿ�� �� ���� (ù��° ���)
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

	// ���� �� �����Ͱ� ���� ������ ��ٸ��� ����
	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(m_mutex);	// �߰��� lock�� Ǯ��� �ϹǷ� unique_lock
		m_cv.wait(lock, [this] {return m_stack.empty() == false; });	// ���� ��� ���� ������ �������� ������ lock ����
		value = std::move(m_stack.top());
		m_stack.pop();
	}

	// ũ�� �ǹ̰� ����.
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