#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "ThreadManager.h"

#include "PlayerMananger.h"
#include "AccountManager.h"

int main()
{
	g_threadManager->Launch([=] {
		while (true) {
			cout << "PlayerThenAccount" << endl;
			g_playerManager.PlayerThenAccount();
			this_thread::sleep_for(100ms);
		}
	});

	g_threadManager->Launch([=] {
		while (true) {
			cout << "AccountThenPlayer" << endl;
			g_accountManager.AccountThenPlayer();
			this_thread::sleep_for(100ms);
		}
		});

	g_threadManager->Join();
}