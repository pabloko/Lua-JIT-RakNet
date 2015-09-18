#include "stdafx.h"

#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include "../includes/RakNet/TCPInterface.h"
#include "../includes/RakNet/RakPeerInterface.h"
#include "../includes/RakNet/MessageIdentifiers.h"
#include "../includes/RakNet/BitStream.h"
#include "../includes/RakNet/RakNetTypes.h"
#include "../includes/LuaJIT/src/lua.hpp"

#include "../RakNetLua-Shared/LuaFunctions.hpp"

lua_State *L;

static int Log(lua_State* ls)
{
	const char * str = lua_tostring(L, 1);
	m_printf("%s", str);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	isExecutable = true;
	printf("   RakNetLua server 1.0\n");
	printf("   By Pabloko - pbl@0x911.ml");
	f = fopen("server.log", "a");
	Sleep(1500);
	system("cls");
	srand(time(NULL));
	L = lua_open();
	luaL_openlibs(L);
	RegisterTypes(L);
	const luaL_reg tab_funcs_raknet[] = {
		{ "OpenServer", lua_openserver },
		{ "OpenClient", lua_openclient },
		{ "Disconnect", lua_disconnect },
		{ "Suscribe", lua_NetSuscribe },
		{ "Send", NetSend },
		{ "Ban", Ban },
		{ "Kick", Kick },
		{ "GetUserNetwork", lua_getusernetwork },
		{ NULL, NULL }
	};
	const luaL_reg tab_funcs_bitstream[] = {
		{ "Create", NetCreateBitStream },
		{ "Write", NetWriteBitStream },
		{ "Read", NetReadBitStream },
		{ NULL, NULL }
	};
	luaL_openlib(L, "RakNet", tab_funcs_raknet, NULL);
	luaL_openlib(L, "BitStream", tab_funcs_bitstream, NULL);
	lua_register(L, "Log", Log);
	sprintf(szCmd, "");
	if (luaL_dofile(L, "server.lua"))
		m_printf("%s", lua_tostring(L, -1));
	getch();
	fclose(f);
	Disconnect();
	return 0;
}

