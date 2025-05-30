local toolbox = loadstring(game:HttpGet("https://raw.githubusercontent.com/prcw/athena/refs/heads/main/toolbox.lua"))()
local gs = toolbox.gs

local players = gs("Players")
local local_player = players.LocalPlayer
local character = local_player.Character or local_player.CharacterAdded:Wait()
local humanoid = character:FindFirstChild("Humanoid")

local replicated_storage = gs("ReplicatedStorage")
local character_sound_event = replicated_storage:WaitForChild("Remotes"):FindFirstChild("CharacterSoundEvent")

local run_service = gs("RunService")
local user_input_service = gs("UserInputService")
