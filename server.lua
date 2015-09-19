Log("   <=============================================>")
Log("   <=========== Simple server started ===========>")
Log("   <=============================================>")

--Network packets definition ===========================================>

ID_NEW_INCOMING_CONNECTION = 19
ID_DISCONNECTION_NOTIFICATION = 21
ID_CONNECTION_LOST = 22
------------------------------------------------------------------------>
ID_NICK_NAME = 135
ID_MAKEMOVE = 136
ID_STARTGAME = 137
ID_ENDGAME = 138
ID_CHAT = 139
ID_COMMAND = 140
ID_ADVERTISE = 141
ID_GONE = 142

--Global Definitions =================================================>

Users={}

-- Cmd Commds ========================================================>

onCmd = function(szCmd)
	local sCommand = DelimitedStringToTable(szCmd," ")
	if (string.lower(sCommand[1])=="kick" and #sCommand == 2) then
		if (Users[tonumber(sCommand[2])] ~= nil) then
			RakNet.Kick(tonumber(sCommand[2]));
			table.remove(Users, tonumber(sCommand[3]))
			local bsOut = BitStream.Create(ID_GONE)
			BitStream.Write(bsOut, tonumber(sCommand[2]), TUINT8)
			for Key,User in pairs(Users) do
				RakNet.Send(bsOut, Key)
			end
		end
	elseif (string.lower(sCommand[1])=="ban" and #sCommand == 3) then
		if (Users[tonumber(sCommand[2])] ~= nil) then
			RakNet.Ban(tonumber(sCommand[2]), tonumber(sCommand[3]));
			table.remove(Users, tonumber(sCommand[3]))
			local bsOut = BitStream.Create(ID_GONE)
			BitStream.Write(bsOut, tonumber(sCommand[2]), TUINT8)
			for Key,User in pairs(Users) do
				RakNet.Send(bsOut, Key)
			end
		end
	else 
		--Just send the text to all players
		for Key,User in pairs(Users) do
			Broadcast(szCmd, Key)
		end
	end
end

-- HTTP Interface ====================================================>

onHTTP = function(headers, ip)
	Log("*** HTTP request from: "..ip)
	playerlist=""
	for Key,User in pairs(Users) do
			if (Key ~= Sender) then
				playerlist=playerlist.."<br>"..User['NickName'].." ("..Key..")", Sender
			end
		end
	return "<html><head><title>AYY LMAO 1.0 By Pabloko</title></head><body style='background-image: url(http://i2.kym-cdn.com/photos/images/newsfeed/000/632/652/6ca.jpg); background-size: cover;'><h1>Ayy Lmaoo Server</h1><hr>Request:<br>"..headers.."<hr>Players:"..playerlist.."</body></html>"
end

-- Packet Interface ==================================================>


RakNet.Suscribe(ID_NEW_INCOMING_CONNECTION, function (Sender, bsIn)
	local ip, port = RakNet.GetUserNetwork(Sender)
	Log(">>> New connection ("..Sender..") From: "..ip..":"..port)
end);

RakNet.Suscribe(ID_DISCONNECTION_NOTIFICATION, function (Sender, bsIn)
	--@TODO: handle exit while playing or /retar
	--Remove user from table
	table.remove(Users, Sender)
	--Stream disconnection to all connected users
	local bsOut = BitStream.Create(ID_GONE)
	BitStream.Write(bsOut, Sender, TUINT8)
	for Key,User in pairs(Users) do
		RakNet.Send(bsOut, Key)
	end
	Log(">>> Disconnected ("..Sender..")")
end);

RakNet.Suscribe(ID_CONNECTION_LOST, function (Sender, bsIn)
	--@TODO: handle exit while playing or /retar
	table.remove(Users, Sender)
	local bsOut = BitStream.Create(ID_GONE)
	BitStream.Write(bsOut, Sender, TUINT8)
	for Key,User in pairs(Users) do
		RakNet.Send(bsOut, Key)
	end
	Log(">>> Timed Out ("..Sender..")")
end);

--Game Logic

RakNet.Suscribe(ID_NICK_NAME, function (Sender, bsIn)
	--Get Nickname
	local sName = BitStream.Read(bsIn, TSTRING)
	--Stream id and nickname to all connected users
	local bsOut = BitStream.Create(ID_NICK_NAME)
	BitStream.Write(bsOut, Sender, TUINT8)
	BitStream.Write(bsOut, sName, TSTRING)
	for Key,User in pairs(Users) do
		if (Key ~= Sender) then
			RakNet.Send(bsOut, Key)
		end
	end
	-- Stream to newly connected user all players connected
	for Key,User in pairs(Users) do
		local bsOut = BitStream.Create(ID_NICK_NAME)
		BitStream.Write(bsOut, Key, TUINT8)
		BitStream.Write(bsOut, User['NickName'], TSTRING)
		RakNet.Send(bsOut, Sender)
	end
	--Save our user to users table with default data
	Users[Sender]={['NickName'] = sName, ['ChatRoom'] = 0, ['Opponent'] = -1, ['Playing'] = false }
	Log("*** Nick choosed: "..sName.." ("..Sender..")")
end);

RakNet.Suscribe(ID_CHAT, function (Sender, bsIn)
	--Recieve chat text
	local sText = BitStream.Read(bsIn, TSTRING)
	Log("*** [CHAT] "..Users[Sender]['NickName'].." ("..Sender.."): "..sText)
	--Stream this message to all other connected users in the same ChatRoom
	for Key,User in pairs(Users) do
		local bsOut = BitStream.Create(ID_CHAT)
		BitStream.Write(bsOut, Sender, TUINT8)
		BitStream.Write(bsOut, sText, TSTRING)
		if ( User['ChatRoom'] == Users[Sender]['ChatRoom'] and Key ~= Sender ) then
			RakNet.Send(bsOut, Key)
		end
	end
end);

RakNet.Suscribe(ID_MAKEMOVE, function (Sender, bsIn)
	--Read column and row pos
	local uCol = BitStream.Read(bsIn, TUINT8)
	local uRow = BitStream.Read(bsIn, TUINT8)
	--Send to the opponent
	local bsOut = BitStream.Create(ID_MAKEMOVE)
	BitStream.Write(bsOut, uCol, TUINT8)
	RakNet.Send(bsOut, Users[Sender]['Opponent'])
end);

RakNet.Suscribe(ID_ENDGAME, function (Sender, bsIn)
	--Read match state
	local bWin = BitStream.Read(bsIn, TBOOL)
	--Broadcast to opponent and self
	local bsOut1 = BitStream.Create(ID_ENDGAME)
	BitStream.Write(bsOut1, not bWin, TBOOL)
	RakNet.Send(bsOut1, Users[Sender]['Opponent'])
	local bsOut2 = BitStream.Create(ID_ENDGAME)
	BitStream.Write(bsOut2, bWin, TBOOL)
	RakNet.Send(bsOut2, Sender)
	--Cleanup vars
	Users[Users[Sender]['Opponent']]['ChatRoom']=0;
	Users[Users[Sender]['Opponent']]['Opponent']=-1;
	Users[Sender]['ChatRoom']=0;
	Users[Sender]['Opponent']=-1;
end);

RakNet.Suscribe(ID_COMMAND, function (Sender, bsIn)
	--Recieve command text
	local sText = BitStream.Read(bsIn, TSTRING)
	--Split command by space delimiter
	local sCommand = DelimitedStringToTable(sText," ")
	--Compare lowercase command
	if (string.lower(sCommand[1])=="/retar") then
		--Has 2 arguments?
		if (#sCommand ~= 2) then
			Broadcast("/retar [id]", Sender)
		else
			--Is number
			local oid = tonumber(sCommand[2])
			if (oid~=nil) then
				--User is online?
				if (Users[oid]~=nil) then
					--User is free?
					if (Users[oid]['Opponent'] == -1) then
						Users[Sender]['Opponent'] = oid
						Users[oid]['Opponent'] = Sender
						Broadcast("*** "..Users[oid]['NickName'].." ("..oid..") ha sido retado /cancelar", Sender)
						Broadcast("*** "..Users[Sender]['NickName'].." ("..Sender..") te ha retado /aceptar o /cancelar", oid)
					else
						Broadcast("*** Usuario retado o jugando en este momento", Sender)
					end
				else
					Broadcast("*** ID de usuario incorrecto", Sender)
				end
			else
				Broadcast("*** ID de usuario incorrecto", Sender)
			end
		end
	end
	
	--Check /cancelar
	if (string.lower(sCommand[1])=="/cancelar") then
		--Sender have opponent?
		if (Users[Sender]['Opponent'] > -1) then
			--Opponent have sender as opponent?
			if (Users[Users[Sender]['Opponent']]['Opponent'] == Sender) then
				Broadcast("*** ".. Users[Sender]['NickName'].." ("..Sender..") Ha cancelado el reto", Users[Sender]['Opponent'])
				Users[Users[Sender]['Opponent']]['Opponent']=-1;
			end
			Users[Sender]['Opponent']=-1;
			Broadcast("*** Retos cancelados", Sender)
		end
	end
	
	--Check /aceptar
	if (string.lower(sCommand[1])=="/aceptar") then
		--Sender have opponent?
		if (Users[Sender]['Opponent'] > -1) then
			--Opponent have sender as oppoent
			if (Users[Users[Sender]['Opponent']]['Opponent'] == Sender) then
				Broadcast("*** Joining the game...", Sender)
				Broadcast("*** Joining the game...", Users[Sender]['Opponent'])
				local bsOut1 = BitStream.Create(ID_STARTGAME)
				BitStream.Write(bsOut1, Sender, TUINT8)
				BitStream.Write(bsOut1, 0, TBOOL)
				RakNet.Send(bsOut1, Users[Sender]['Opponent'])
				local bsOut2 = BitStream.Create(ID_STARTGAME)
				BitStream.Write(bsOut2, Users[Sender]['Opponent'], TUINT8)
				BitStream.Write(bsOut2, 1, TBOOL)
				RakNet.Send(bsOut2, Sender)
				--Change room
				Users[Sender]['ChatRoom'] = Sender + 100
				Users[Users[Sender]['Opponent']]['ChatRoom'] = Sender + 100
			end
		end
	end
	
	--Check /join
	if (string.lower(sCommand[1])=="/join") then
		--Has 2 arguments?
		if (#sCommand ~= 2) then
			Broadcast("/join [0-99]", Sender)
		else
			--Is number
			local oid = tonumber(sCommand[2])
			if (oid ~= nil) then
				if ( oid >= 0 and oid < 100 ) then
				Users[Sender]['ChatRoom'] = Sender
				end
			end
		end
	end
	
	--Check /pm
	if (string.lower(sCommand[1])=="/pm") then
		--Has 2 arguments?
		if (#sCommand ~= 3) then
			Broadcast("/pm [id] [msg]", Sender)
		else
			local oid = tonumber(sCommand[2])
			if (oid ~= nil) then
				if ( Users[oid] ~= nil ) then
					sCommand[1]=''; sCommand[2]='';
					local sText = table.concat(sCommand, " ")
					Broadcast("[PM] "..Users[Sender]['NickName'].." ("..Sender.."):"..sText ,oid)
				end
			end
		end
	end
	
	--Check /users
	if (string.lower(sCommand[1])=="/users") then
		Broadcast("=== User list ===", Sender)
		for Key,User in pairs(Users) do
			if (Key ~= Sender) then
				Broadcast(User['NickName'].." ("..Key..")", Sender)
			end
		end
		Broadcast("=== User list ===", Sender)
	end
	
end);


-- Open the server  =================================================>

-- PORT, MAX_CONNECTIONS, PASSWORD, MULTITHREADED|SINGLETHREADED (single will need Update calls)
RakNet.OpenServer(1337, 500, "", SINGLETHREADED)

-- Helper functions =================================================>

local ffi = require("ffi")
ffi.cdef[[
int MessageBoxA(void *w, const char *txt, const char *cap, int type);
void Sleep(int ms);
]]

Sleep = function(ms)
	ffi.C.Sleep(ms)
end

MessageBox = function(title, text)
	ffi.C.MessageBoxA(nil, text, title, 0)
end

function Broadcast(sText, To)
	local bsOut = BitStream.Create(ID_ADVERTISE)
	BitStream.Write(bsOut, sText, TSTRING)
	RakNet.Send(bsOut, To)
end

function DelimitedStringToTable(s, d)
        if not d or #d < 1 then return nil end
        local tbl = {};
        local sa = s;
        local sD = "";
        local nP = string.find(sa, d, 1, true)
        while nP do
                sD = string.sub(sa, 1, nP-1)
                table.insert(tbl, #tbl+1, sD)
                sa = string.sub(sa, nP+1, -1)
                nP = string.find(sa, d, 1, true)
        end
        if sa ~= "" then table.insert(tbl, #tbl+1, sa) end
        return tbl;
end

-- Main =============================================================>

MessageBox("server","Starting...")
while (true) do
	RakNet.Update()
	Sleep(20)
end
