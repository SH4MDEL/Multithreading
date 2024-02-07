#include "pch.h"
#include "PlayerMananger.h"
#include "AccountManager.h"

PlayerMananger g_playerManager;

void PlayerMananger::PlayerThenAccount()
{
	WRITE_LOCK;
	g_accountManager.Lock();
}

void PlayerMananger::Lock()
{
	WRITE_LOCK;
}
