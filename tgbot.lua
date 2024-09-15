#!/usr/bin/lua
local args = { ... }
local token = args[1]

local tg = require('telegram-bot-lua.core').configure(token)

function broadcast(user, chat, service, text) -- impure
	print(user, chat, service, text)
end

function tg.on_message(message) -- impure
	if message.text then
		broadcast(message.from.first_name, message.chat.title, "tg", message.text)
	end
end

tg.run()
