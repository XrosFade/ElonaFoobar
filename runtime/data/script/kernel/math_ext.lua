local math = require "math"

local Math = {}

Math.min = math.min
Math.max = math.max
Math.sqrt = math.sqrt
Math.modf = math.modf

function Math.clamp(n, min, max)
  return Math.min(Math.max(n, min), max)
end

return Math
