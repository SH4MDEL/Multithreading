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
	// ��尡 �߰ߵ� ������ ����ϴ� �迭.
	int32								m_discoveredCount = 0;
	// ��尡 �߰ߵ� ����
	vector<bool>						m_finished;
	// Dfs(i)�� ����Ǿ����� ����
	vector<int32>						m_parent;
};

