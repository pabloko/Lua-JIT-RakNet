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

extern "C" int __declspec(dllexport) luaopen_RakNetLua(lua_State* L);
int luaopen_RakNetLua(lua_State* L)
{
	isExecutable = false;
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
		{ "GetUserNetwork", lua_getusernetwork },
		{ NULL, NULL }
	};
	luaL_openlib(L, "RakNet", tab_funcs, NULL);
	return 1;
}