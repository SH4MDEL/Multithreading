#include "pch.h"
#include "DeadLockProfiler.h"

/*
 *	DeadLock Profiler
 */

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(m_lock);

	// ���̵� ã�ų� �߱��Ѵ�.
	int32 lockId = 0;

	auto findIter = m_nameToId.find(name);
	if (findIter == m_nameToId.end()) {
		// ���� �ش� Lock�� �������� �ʴ´ٸ�, �� ID�� �ο��Ѵ�.
		lockId = static_cast<int32>(m_nameToId.size());
		m_nameToId[name] = lockId;
		m_idToName[lockId] = name;
	}
	else {
		// �����Ѵٸ�, �� ID�� �����´�.
		lockId = findIter->second;
	}

	// ��� �ִ� Lock�� �־��ٸ�
	if (!m_lockStack.empty()) {
		// ������ �߰ߵ��� ���� ���̽���� ������� �ִ���(����Ŭ�� �ִ���) Ȯ���ؾ� �Ѵ�.
		const int32 prevId = m_lockStack.top();
		if (lockId != prevId) {
			// (Recursive�� �����ϹǷ�) ���� Lock�� ��� ���� ����Ѵ�.
			set<int32>& history = m_lockHistory[prevId];
			if (history.find(lockId) == history.end()) {
				// ó�� �߰��� ID�� ���(���ο� ������ �߰����� ���) ����Ŭ�� üũ�Ѵ�. 
				history.insert(lockId);
				CheckCycle();
			}

		}
	}

	m_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(m_lock);

	if (m_lockStack.empty()) {
		CRASH("MULTIPLE_UNLOCK");
	}

	int32 lockId = m_nameToId[name];
	if (m_lockStack.top() != lockId) {
		CRASH("INVALID_UNLOCK");
	}

	m_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(m_nameToId.size());
	m_discoveredOrder = vector<int32>(lockCount, -1);
	m_discoveredCount = 0;
	m_finished = vector<bool>(lockCount, false);
	m_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; ++lockId) {
		Dfs(lockId);
	}

	// ���� ���� �� ����
	m_discoveredOrder.clear();
	m_finished.clear();
	m_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (m_discoveredOrder[here] != -1) {
		return;
	}

	m_discoveredOrder[here] = ++m_discoveredCount;

	// ��� ������ ������ ��ȸ�Ѵ�.
	auto findIter = m_lockHistory.find(here);
	if (findIter == m_lockHistory.end()) {
		m_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIter->second;
	for (int32 there : nextSet) {
		// ���� �湮�� ���� ���ٸ� �湮�Ѵ�.
		if (m_discoveredOrder[there] == -1) {
			// here�� ���� �湮������ ǥ���Ѵ�.
			m_parent[there] = here;
			Dfs(there);
			continue;
		}

		// �̹� �湮�� ���� �ִٸ� ���������� ���������� Ȯ���Ѵ�.
		// here�� there���� ���� �߰ߵǾ��ٸ�, there�� here�� �ļ��̴�. (������ ����)
		if (m_discoveredOrder[here] < m_discoveredOrder[there]) {
			// �׷��ٸ� ��ȯ ��� ������ �������� �����Ƿ� �Ѿ�� ����.
			continue;
		}

		// �������� �ƴϰ�, Dfs(there)�� ���� �������� �ʾҴٸ�, there�� here�� �����̴�.
		// (������ ������ �߰ߵǾ���.)
		if (m_finished[there] == false) {
			printf("%s -> %s\n", m_idToName[here], m_idToName[there]);

			int32 now = here;
			while (true) {
				printf("%s -> %s\n", m_idToName[m_parent[now]], m_idToName[now]);
				now = m_parent[now];
				if (now == there) {
					break;
				}
			}
			CRASH("DEADLOCK_DETECTED");
		}
	}

	m_finished[here] = true;
}
