_defaultParams = {
	{"pulse width",255},
	{"num bits",8},
}

function _init()
	return{0,0,true,0}
end

function next_integrated_blep_sample(t)
    local t1 = 0.5 * t
    local t2 = t1 * t1
    local t4 = t2 * t2
    return 0.1875 - t1 + 1.5 * t2 - t4
end

#[inline]
function this_integrated_blep_sample(t)
    return next_integrated_blep_sample(1 - t)
end

function _audioFrame(synthData)
	local synthData = synthData
	local phase = synthData[1]
	local step = synthData[2]
	local ascending = synthData[3]
	local nextSample = synthData[4]

	local num_steps = 1 << C.konGet(1);
    local half = num_steps / 2;
    local top = num_steps - 1
    if num_steps == 2 then top = 2 end
    local scale = 4.0 / (top - 1)
    if num_steps == 2 then scale = 2 end

	local frequency = C.konGet(0)/sampleRate

	local thisSample = nextSample
	nextSample = 0

	phase=phase+frequency
	phase=phase%1

	synthData[1] = phase
	synthData[2] = step
	synthData[3] = ascending
	synthData[4] = nextSample

	if ascending && phase >= .5 then
		local discontinuity = 4 * frequency
		if discontinuity ~= 0 then
			local t = (phase - .5) / frequency
			thisSample = thisSample - this_integrated_blep_sample(t) * discontinuity
			nextSample = nextSample - next_integrated_blep_sample(t) * discontinuity
		end
		ascending = false
	end

	local nextStep = (phase * numStepsF)

	if nextStep ~= step then

	end

	local out=phase

	if(out > C.konGet(3)/510)then
		out=1
	else
		out=-1
	end

	C.konOut(out,out)
end