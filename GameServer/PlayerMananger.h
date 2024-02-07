#pragma once

class PlayerMananger
{
	USE_LOCK;

public:
	void PlayerThenAccount();
	void Lock();
};

extern PlayerMananger g_playerManager;

