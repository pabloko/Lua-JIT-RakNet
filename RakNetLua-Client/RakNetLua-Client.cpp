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

extern "C" int __declspec(dllexport) luaopen_RakNetLua(lua_State* L);
int luaopen_RakNetLua(lua_State* L)
{
	isExecutable = false;
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
	return -1;
}