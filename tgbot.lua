#!/usr/bin/lua
local args = { ... }
local token = args[1]

local tg = require('telegram-bot-lua.core').configure(token)

function tg.on_channel_post(channel_post) -- impure
	if channel_post.text and channel_post.text:match('ping') then
		tg.send_message(channel_post.chat.id, "pong")
	end
end
