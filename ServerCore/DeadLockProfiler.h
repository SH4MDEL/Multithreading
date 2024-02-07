#pragma once
#include <stack>
#include <map>
#include <vector>

/*
 *	DeadLock Profiler
 */

class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	unordered_map<const char*, int32>	m_nameToId;
	unordered_map<int32, const char*>	m_idToName;
	stack<int32>						m_lockStack;
	map<int32, set<int32>>				m_lockHistory;

	Mutex								m_lock;

private:
	vector<int32>						m_discoveredOrder;
	// 노드가 발견된 순서를 기록하는 배열.
	int32								m_discoveredCount = 0;
	// 노드가 발견된 순서
	vector<bool>						m_finished;
	// Dfs(i)가 종료되었는지 여부
	vector<int32>						m_parent;
};

