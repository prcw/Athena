local supported_places = {
	[8204899140] = "Football Fusion 2",
	[17541256525] = "Pro Servers",
	[8206123457] = "New Fusion Practice"
}

local place = supported_places[game.PlaceId]

if not place then return {} end
if not firetouchinterest then return {} end

local toolbox = {}

toolbox.place = place

toolbox.gs = function(sv)
	return game:GetService(sv)
end

return toolbox
