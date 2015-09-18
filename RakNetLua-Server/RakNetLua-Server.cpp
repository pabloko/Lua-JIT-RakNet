#include "stdafx.h"

#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include "../includes/RakNet/RakPeerInterface.h"
#include "../includes/RakNet/MessageIdentifiers.h"
#include "../includes/RakNet/BitStream.h"
#include "../includes/RakNet/RakNetTypes.h"
#include "../includes/LuaJIT/src/lua.hpp"

#include "../RakNetLua-Shared/LuaFunctions.hpp"

lua_State *L;
FILE *f;
char szCmd[256];
char m_pBuffer[1024];

int m_printf(char *fmt, ...)
{
	int ret;
	va_list myargs;
	va_start(myargs, fmt);
	ret = vsprintf(m_pBuffer, fmt, myargs);
	printf(m_pBuffer);
	if (f != NULL)
		fprintf(f, m_pBuffer);
	va_end(myargs);
	return ret;
}

//Log a string to stdout and file
static int Log(lua_State* ls)
{
	const char * str = lua_tostring(L, 1);
	m_printf("%s\n", str);
	return 0;
}


//Alternative and simpler thread to run a default "main"
int LuaThread(void) {
	while (1) {
		lua_getglobal(L, "main");
		if (lua_isfunction(L, -1))
		{
			if (lua_pcall(L, 0, 0, 0) != 0) {
				m_printf("%s\n", lua_tostring(L, -1));
				return 0;
			}
		}
		else {
			lua_remove(L, -1);
			return 0;
		}
	}
	return 0;
}

//Load script file
void ExecuteServerRoutine() {
	if (luaL_dofile(L, "server.lua"))
		printf("%s\n", lua_tostring(L, -1));
}

int _tmain(int argc, _TCHAR* argv[])
{
	isExecutable = true;
	printf("   RakNetLua server 1.0\n");
	printf("   By Pabloko - pbl@0x911.ml");
	//Prepare file to store logs
	f = fopen("server.log", "a");
	Sleep(1500);
	system("cls");
	srand(time(NULL));
	L = lua_open();
	luaL_openlibs(L);
	RegisterTypes(L);
	const luaL_reg tab_funcs[] = {
		{ "OpenServer", lua_openserver },
		{ "OpenClient", lua_openclient },
		{ "Disconnect", lua_disconnect },
		{ "Send", NetSend },
		{ "CreateBitStream", NetCreateBitStream },
		{ "CreateBitStream", NetCreateBitStream },
		{ "WriteBitStream", NetWriteBitStream },
		{ "ReadBitStream", NetReadBitStream },
		{ "Ban", Ban },
		{ "Kick", Kick },
		{ "Log", Log },
		{ "GetUserNetwork", lua_getusernetwork },
		{ NULL, NULL }
	};
	luaL_openlib(L, "RakNet", tab_funcs, NULL);
	sprintf(szCmd, "");
	ExecuteServerRoutine();
	HANDLE ThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LuaThread, 0, 0, 0);
	while (1)
	{
		if (peer != NULL) {
			for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
				//lua_getglobal(L, "NetProcess");
				lua_rawgeti(L, LUA_REGISTRYINDEX, pRemoteFn[packet->data[0]]);
				lua_pushnumber(L, packet->data[0]);
				lua_gettable(L, -2);
				lua_remove(L, -2);
				if (lua_isfunction(L, -1))
				{
					lua_pushinteger(L, packet->systemAddress.systemIndex + 1);
					RakNet::BitStream stream(packet->data, packet->length, false);
					stream.IgnoreBytes(sizeof(RakNet::MessageID));
					lua_pushlightuserdata(L, &stream);
					if (lua_pcall(L, 2, 0, 0) != 0)
						m_printf("%s\n", lua_tostring(L, -1));
				}
				else
					lua_remove(L, -1);
			}
		}
		//Reload F11+F12
		if (GetAsyncKeyState(VK_F12) && GetAsyncKeyState(VK_F11))
		{
			sprintf(szCmd, "");
			TerminateThread(ThreadHandle, 0);
			peer->Shutdown(100);
			system("cls");
			Sleep(300);
			RakNet::RakPeerInterface::DestroyInstance(peer);
			ExecuteServerRoutine();
			ThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LuaThread, 0, 0, 0);
		}

		//Input
		int ch;
		while (_kbhit())
		{
			ch = _getch();
			if (ch == 0x0D) {
				m_printf("\n");
				lua_getglobal(L, "onCmd");
				if (lua_isfunction(L, -1))
				{
					lua_pushstring(L, szCmd);
					if (lua_pcall(L, 1, 0, 0) != 0)
						m_printf("%s\n", lua_tostring(L, -1));
				}
				else
					lua_remove(L, -1);
				sprintf(szCmd, "");
			}
			else {
				m_printf("%c", ch);
				sprintf(szCmd, "%s%c", szCmd, ch);
			}
		}
	}
	fclose(f);
	peer->Shutdown(100);
	RakNet::RakPeerInterface::DestroyInstance(peer);
	return 0;
}

