EverGrandeCity_Hall3_MapScripts:: @ 8229593
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, EverGrandeCity_Hall3_OnWarp
	.byte 0

EverGrandeCity_Hall3_OnWarp: @ 8229599
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_Hall3_EventScript_TurnPlayerNorth
	.2byte 0

EverGrandeCity_Hall3_EventScript_TurnPlayerNorth:: @ 82295A3
	turnobject EVENT_OBJ_ID_PLAYER, DIR_NORTH
	gfec
	setobjectxy EVENT_OBJ_ID_FOLLOWER, VAR_TEMP_1, VAR_TEMP_2
	end

