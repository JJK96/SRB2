local username = {}
local skin = {}
local time = {}
local full_rows = 0
local used_row = 0
local no_time = true

local function receive_data(source, type, target, msg)
	if msg:sub(0, 5) == "UXDFS"
		local data_msg = msg:sub(6)
		
		full_rows = full_rows + 1
		
		local first_comma = data_msg:find(",")
		local second_comma = data_msg:find(",", first_comma+1)
		
		username[full_rows] = data_msg:sub(0, first_comma-1)
		skin[full_rows] = data_msg:sub(first_comma+1, second_comma-1)
		time[full_rows] = data_msg:sub(second_comma+1)
		return true
	end
end
addHook("PlayerMsg", receive_data)


local function reset_on_map_change(mapnum)
	username = {}
	skin = {}
	time = {}
	full_rows = 0
	used_row = 0
end
addHook("MapLoad", reset_on_map_change)


local function find_in_array(array, value)
	for k, v in ipairs(array) do
		if v == value
			return k
		end
	end
	return -1
end


local function check_player_skin(player)
	if player == consoleplayer
		used_row = find_in_array(skin, player.mo.skin)
		if used_row == -1
			no_time = true
		else
			no_time = false
		end
	end
end
addHook("PlayerThink", check_player_skin)


local function show_score(v)
	if no_time
		v.drawString(4, 173, "NO BEST TIME YET, BE FIRST TO FINISH!", 45056)
	else
		v.drawString(4, 173, "BEST TIME FOR " + string.upper(skin[used_row]) + ":", 45056)
		v.drawString(4, 182, time[used_row]+" by "+username[used_row])
	end
	
end
hud.add(show_score, "scores")
