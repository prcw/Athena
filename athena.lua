_G.athena = {
	magnets = {
		radius = 15
	},
	catch_resizer = {
		size = Vector3.new(0, 0, 0)
	},
	follow = {
		mode = "DE",
		
		keyboard = {
			bind = "C",
			mode = "Hold",
			
			safety = {
				input = false
			}
		}
	}, 
	movement = {
		angle = {
			use = true,
			height = 0,
			
			safety = {
				active_s = false
			}
		},
		
		freeze = {
			use = true,
			
			keyboard = {
				bind = "T",
				mode = "Hold"
			}
		}
	},
	gamepasses = {
		uniform = {
			name = "",
			number = 0,
			gloves = false
		}
	}
}

local athena = _G.athena

local magnets = athena.magnets
local catch_resizer = athena.catch_resizer

local follow = athena.follow
local follow_keyboard = follow.keyboard

local movement = athena.movement
local angle = movement.angle

local freeze = movement.freeze
local freeze_keyboard = freeze.keyboard

local gamepasses = athena.gamepasses
local uniform = gamepasses.uniform
