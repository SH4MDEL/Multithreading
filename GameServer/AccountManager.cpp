#include "pch.h"
#include "AccountManager.h"
#include "PlayerMananger.h"

AccountManager g_accountManager;

void AccountManager::AccountThenPlayer()
{
	WRITE_LOCK;
	g_playerManager.Lock();
}

void AccountManager::Lock()
{
	WRITE_LOCK;
}
