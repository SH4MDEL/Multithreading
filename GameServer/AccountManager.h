#pragma once
class AccountManager
{
	USE_LOCK;

public:
	void AccountThenPlayer();
	void Lock();
};

extern AccountManager g_accountManager;