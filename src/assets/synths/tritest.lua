--test of porting a part of plaits
--ended up being pretty lame, because i purposefully chose a simple one 

_defaultParams = {
	{"num bits",8},
}

function _init()
	return{0,0,true,0}
end

function this_blep_sample(t)
    return 0.5 * t * t
end

function next_blep_sample(t)
    local t = 1.0 - t
    return -0.5 * t * t
end

function next_integrated_blep_sample(t)
    local t1 = 0.5 * t
    local t2 = t1 * t1
    local t4 = t2 * t2
    return 0.1875 - t1 + 1.5 * t2 - t4
end

function this_integrated_blep_sample(t)
    return next_integrated_blep_sample(1 - t)
end

function _audioFrame(synthData)
	local phase = synthData[1]
	local step = synthData[2]
	local ascending = synthData[3]
	local nextSample = synthData[4]

	local numSteps = bit.lshift(1,floor(C.konGet(3)*5+3))--2^(C.konGet(3)*5+3)
    local half = numSteps / 2
    local top = numSteps - 1
    if numSteps == 2 then top = 2 end
    local scale = 4.0 / (top - 1)
    if numSteps == 2 then scale = 2 end

	local frequency = C.konGet(0)/sampleRate

	phase=phase+frequency

	local fadeToTri = ((frequency - 0.5 / numSteps) * 2.0 * numSteps)
	--no clamp yet
	if fadeToTri > 1 then fadeToTri = 1 end
	if fadeToTri < 0 then fadeToTri = 0 end

	local nesGain = 1 - fadeToTri
	local triGain = fadeToTri * 2 / scale

	local thisSample = nextSample
	nextSample = 0

	if ascending and phase >= .5 then
		local discontinuity = 4 * frequency
		if discontinuity ~= 0 then
			local t = (phase - .5) / frequency
			thisSample = thisSample - this_integrated_blep_sample(t) * discontinuity
			nextSample = nextSample - next_integrated_blep_sample(t) * discontinuity
		end

		ascending = false
	end

	local nextStep = floor(phase * numSteps)

	if nextStep ~= step then
		local wrap = false

		if nextStep >= numSteps then
			phase = phase - 1
			nextStep = nextStep - numSteps
			wrap = true
		end

		local discontinuity = (nextStep < half) and 1 or -1

		if numSteps == 2 then
			discontinuity = -discontinuity
		elseif nextStep == 0 or nextStep == half then
			discontinuity = 0
		end

		discontinuity = discontinuity * nesGain

		if discontinuity ~= 0 then
			local frac = phase * numSteps - nextStep
			local t = frac / (frequency * numSteps)
			thisSample = thisSample + this_blep_sample(t) * discontinuity
			nextSample = nextSample + next_blep_sample(t) * discontinuity
		end

		if wrap then
			local discontinuity = 4 * frequency * triGain
			if discontinuity ~= 0 then
				local t = phase / frequency
				thisSample = thisSample + this_integrated_blep_sample(t) * discontinuity
				nextSample = nextSample + next_integrated_blep_sample(t) * discontinuity
			end
			ascending = true
		end
	end

	step = nextStep

	if step < half then
		nesGain = nesGain * step
	else
		nesGain = nesGain * (top - step)
	end

	if phase < .5 then
		triGain = triGain * (2 * phase)
	else
		triGain = triGain * (2 - 2 * phase)
	end

	nextSample = nextSample + nesGain

	nextSample = nextSample + triGain

	local out=thisSample * scale - 1

	synthData[1] = phase
	synthData[2] = step
	synthData[3] = ascending
	synthData[4] = nextSample

	C.konOut(out,out)
end