#!/usr/bin/lua
local args = { ... }
local token = args[1]

local tg = require('telegram-bot-lua.core').configure(token)

function broadcast(user, text) -- impure
	local file = io.open("./bridge", "a+")
	print(user, text)
	file:write(user, "\n", "#botwars", "\n", "tg", "\n", text, "\n", string.char(4), "\n")
	file:close()
end

function tg.on_message(message) -- impure
	if message.text then
		broadcast(message.from.first_name, message.text)
	end
end

--[[function tg.on_update(update)
	local readfile = io.popen("./bridge", "rb")
	local eof = readfile:seek("end")
	for i=1,eof do
		readfile:seek("set",eof-i)
		if i==eof then break end
		if readfile:read(1)=="\n" then break end
	end
	table.insert(readbuffer,readfile:read("*a"))
	readfile:close()
end]]

bot = coroutine.wrap(tg.run)
bot()
