//Types over network
enum NETWORK_TYPES {
	TBOOL = 0,
	TUINT8,
	TINT8,
	TUINT16,
	TINT16,
	TUINT32,
	TINT32,
	TFLOAT,
	TDOUBLE,
	TLONG,
	TSTRING
};

int pRemoteFn[512];

RakNet::RakPeerInterface *peer;
bool isServer;
RakNet::Packet *packet;
char iIP[125];
HANDLE RakThreadHandle;
bool isExecutable;

void RakThread(void * lp)
{
	lua_State* L = (lua_State*)lp;
	do{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			if (pRemoteFn[packet->data[0]] != NULL) {
				//lua_getglobal(L, "NetProcess");
				lua_rawgeti(L, LUA_REGISTRYINDEX, pRemoteFn[packet->data[0]]);
				lua_pushnumber(L, packet->data[0]);
				lua_gettable(L, -2);
				lua_remove(L, -2);
				if (lua_isfunction(L, -1))
				{
					if (isServer)
						lua_pushinteger(L, packet->systemAddress.systemIndex + 1);
					RakNet::BitStream stream(packet->data, packet->length, false);
					stream.IgnoreBytes(sizeof(RakNet::MessageID));
					lua_pushlightuserdata(L, &stream);
					if (lua_pcall(L, 2, 0, 0) != 0)
						lua_remove(L, -1);
				}
				else
					lua_remove(L, -1);
			}
		}
		Sleep(5);
	} while (true);
}

int lua_openclient(lua_State* L)
{
	if (peer != NULL)
	{
		if (!isExecutable)
			TerminateThread(RakThreadHandle, 0);
		peer->Shutdown(100);
		RakNet::RakPeerInterface::DestroyInstance(peer);
	}
	peer = RakNet::RakPeerInterface::GetInstance();
	const char * ip = luaL_checkstring(L, 1);
	int port = luaL_checknumber(L, 2);
	isServer = false;
	RakNet::SocketDescriptor sd;
	peer->Startup(1, &sd, 1);
	sprintf(iIP, "%s", ip);
	const char * passwd = luaL_checkstring(L, 3);
	peer->Connect(iIP, port, passwd, (int)strlen(passwd));
	peer->SetOccasionalPing(true);
	peer->SetUnreliableTimeout(500);
	if (!isExecutable)
		RakThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RakThread, (void *)L, 0, 0);
	return 0;
}

int lua_openserver(lua_State* L)
{
	if (peer != NULL)
	{
		if (!isExecutable)
			TerminateThread(RakThreadHandle, 0);
		peer->Shutdown(100);
		RakNet::RakPeerInterface::DestroyInstance(peer);
	}
	peer = RakNet::RakPeerInterface::GetInstance();
	int port = luaL_checknumber(L, 1);
	int max = luaL_checknumber(L, 2);
	const char * passwd = luaL_checkstring(L, 3);
	isServer = true;
	RakNet::SocketDescriptor sd(port, 0);
	peer->Startup(max, &sd, 1);
	peer->SetIncomingPassword(passwd, (int)strlen(passwd));
	peer->SetMaximumIncomingConnections(max);
	peer->SetUnreliableTimeout(500);
	peer->SetOccasionalPing(true);
	if (!isExecutable)
	RakThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RakThread, (void *)L, 0, 0);
	return 0;
}

int lua_disconnect(lua_State* L)
{
	if (!isExecutable)
		TerminateThread(RakThreadHandle, 0);
	peer->Shutdown(100);
	RakNet::RakPeerInterface::DestroyInstance(peer);
	return 0;
}

int lua_NetSuscribe(lua_State* L)
{
	int iDx = luaL_checknumber(L, 1);
	pRemoteFn[iDx] = luaL_ref(L, LUA_REGISTRYINDEX);
	return 0;
}


//Send a packet to some player
static int NetSend(lua_State* L)
{
	RakNet::BitStream *stream = static_cast<RakNet::BitStream*>(lua_touserdata(L, 1));
	if (isServer)
		peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peer->GetSystemAddressFromIndex(lua_tonumber(L, 2) - 1), false);
	else
		peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, 0, false);
	return 0;
}

//Create packet with packet id
static int NetCreateBitStream(lua_State* L)
{
	RakNet::BitStream *stream = new RakNet::BitStream();
	int v1 = luaL_checknumber(L, 1);
	stream->Write((RakNet::MessageID)v1);
	lua_pushlightuserdata(L, stream);
	return 1;
}

//Write to a bitstream
static int NetWriteBitStream(lua_State* L)
{
	RakNet::BitStream *stream = static_cast<RakNet::BitStream*>(lua_touserdata(L, 1));
	int type = lua_tointeger(L, 3);
	switch (type)
	{
	case NETWORK_TYPES::TBOOL: {
		bool var;
		var = lua_toboolean(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TUINT8: {
		uint8_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TINT8: {
		int8_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TUINT16: {
		uint16_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TINT16: {
		int16_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TUINT32: {
		uint32_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TINT32: {
		int32_t var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TFLOAT: {
		FLOAT var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TDOUBLE: {
		double var;
		var = luaL_checknumber(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TLONG: {
		LONG var;
		var = luaL_checklong(L, 2);
		stream->Write(var);
	} break;
	case NETWORK_TYPES::TSTRING: {
		const char* vStr;
		vStr = luaL_checkstring(L, 2);
		RakNet::RakString str = vStr;
		stream->Write(str);
	} break;
	}
	return 0;
}

//read from a bitstream
static int NetReadBitStream(lua_State* L)
{
	RakNet::BitStream *stream = static_cast<RakNet::BitStream*>(lua_touserdata(L, 1));
	int type = lua_tointeger(L, 2);
	switch (type)
	{
	case NETWORK_TYPES::TBOOL: {
		bool var;
		stream->Read(var);
		lua_pushboolean(L, var);
	} break;
	case NETWORK_TYPES::TUINT8: {
		uint8_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TINT8: {
		int8_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TUINT16: {
		uint16_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TINT16: {
		int16_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TUINT32: {
		uint32_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TINT32: {
		int32_t var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TFLOAT: {
		FLOAT var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TDOUBLE: {
		double var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TLONG: {
		LONG var;
		stream->Read(var);
		lua_pushnumber(L, var);
	} break;
	case NETWORK_TYPES::TSTRING: {
		RakNet::RakString str;
		stream->Read(str);
		lua_pushstring(L, str.C_String());
	} break;
	}
	return 1;
}

//kick a faggot
static int Kick(lua_State* L)
{
	if (isServer) {
		int client = luaL_checknumber(L, 1);
		peer->CloseConnection(peer->GetSystemAddressFromIndex(client - 1), true);
	}
	return 0;
}

//banhammer them all
static int Ban(lua_State* L)
{
	if (isServer) {
		int client = luaL_checknumber(L, 1);
		int time = luaL_checknumber(L, 2) * 1000;
		peer->AddToBanList(peer->GetSystemAddressFromIndex(client).ToString(false), time);
		peer->CloseConnection(peer->GetSystemAddressFromIndex(client - 1), true);
	}
	return 0;
}

static int lua_getusernetwork(lua_State* L)
{
	if (isServer) {
		int client = luaL_checknumber(L, 1);
		if (peer->GetSystemAddressFromIndex(client - 1) != RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
			lua_pushstring(L, peer->GetSystemAddressFromIndex(client - 1).ToString(false));
			lua_pushnumber(L, peer->GetSystemAddressFromIndex(client - 1).GetPort());
		}
		else {
			lua_pushnil(L);
			lua_pushnil(L);
		}
		return 2;
	}
	else {
		return 0;
	}
}

void RegisterTypes(lua_State* L)
{
	//Define TYPES on LUA
	lua_pushnumber(L, NETWORK_TYPES::TBOOL);
	lua_setglobal(L, "TBOOL");
	lua_pushnumber(L, NETWORK_TYPES::TUINT8);
	lua_setglobal(L, "TUINT8");
	lua_pushnumber(L, NETWORK_TYPES::TINT8);
	lua_setglobal(L, "TINT8");
	lua_pushnumber(L, NETWORK_TYPES::TUINT16);
	lua_setglobal(L, "TUINT16");
	lua_pushnumber(L, NETWORK_TYPES::TINT16);
	lua_setglobal(L, "TINT16");
	lua_pushnumber(L, NETWORK_TYPES::TUINT32);
	lua_setglobal(L, "TUINT32");
	lua_pushnumber(L, NETWORK_TYPES::TINT32);
	lua_setglobal(L, "TINT32");
	lua_pushnumber(L, NETWORK_TYPES::TFLOAT);
	lua_setglobal(L, "TFLOAT");
	lua_pushnumber(L, NETWORK_TYPES::TDOUBLE);
	lua_setglobal(L, "TDOUBLE");
	lua_pushnumber(L, NETWORK_TYPES::TLONG);
	lua_setglobal(L, "TLONG");
	lua_pushnumber(L, NETWORK_TYPES::TSTRING);
	lua_setglobal(L, "TSTRING");
}