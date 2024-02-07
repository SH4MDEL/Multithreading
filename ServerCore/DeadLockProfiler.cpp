#include "pch.h"
#include "DeadLockProfiler.h"

/*
 *	DeadLock Profiler
 */

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(m_lock);

	// 아이디를 찾거나 발급한다.
	int32 lockId = 0;

	auto findIter = m_nameToId.find(name);
	if (findIter == m_nameToId.end()) {
		// 현재 해당 Lock이 존재하지 않는다면, 새 ID를 부여한다.
		lockId = static_cast<int32>(m_nameToId.size());
		m_nameToId[name] = lockId;
		m_idToName[lockId] = name;
	}
	else {
		// 존재한다면, 그 ID를 가져온다.
		lockId = findIter->second;
	}

	// 잡고 있는 Lock이 있었다면
	if (!m_lockStack.empty()) {
		// 기존에 발견되지 않은 케이스라면 데드락이 있는지(사이클이 있는지) 확인해야 한다.
		const int32 prevId = m_lockStack.top();
		if (lockId != prevId) {
			// (Recursive가 가능하므로) 같은 Lock을 잡는 것은 허용한다.
			set<int32>& history = m_lockHistory[prevId];
			if (history.find(lockId) == history.end()) {
				// 처음 발견한 ID일 경우(새로운 간선을 발견했을 경우) 사이클을 체크한다. 
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

	// 연산 종료 후 정리
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

	// 모든 인접한 정점을 순회한다.
	auto findIter = m_lockHistory.find(here);
	if (findIter == m_lockHistory.end()) {
		m_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIter->second;
	for (int32 there : nextSet) {
		// 아직 방문한 적이 없다면 방문한다.
		if (m_discoveredOrder[there] == -1) {
			// here로 인해 방문했음을 표시한다.
			m_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 이미 방문한 적이 있다면 순방향인지 역방향인지 확인한다.
		// here가 there보다 먼저 발견되었다면, there는 here의 후손이다. (순방향 간선)
		if (m_discoveredOrder[here] < m_discoveredOrder[there]) {
			// 그렇다면 순환 대기 조건을 만족하지 않으므로 넘어가도 좋다.
			continue;
		}

		// 순방향이 아니고, Dfs(there)가 아직 종료하지 않았다면, there는 here의 선조이다.
		// (역방향 간선이 발견되었다.)
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
