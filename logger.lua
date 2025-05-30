local logger = {}

local prefix = "[athena]"

local function format_message(level, ...)
	local args = table.pack(...)
	for i = 1, args.n do
		args[i] = tostring(args[i])
	end
	return string.format("%s [%s] %s", prefix, level, table.concat(args, " "))
end

function logger.warn(...)
	warn(format_message("warn", ...))
end

function logger.error(...)
	error(format_message("error", ...), 2)
end

function logger.verbose(...)
	print(format_message("verbose", ...))
end

return logger
