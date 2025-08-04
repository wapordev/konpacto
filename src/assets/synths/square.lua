_defaultParams = {
	{"pulse width",255},
}

function _init()
	return{0}
end

function _audioFrame(synthData,data)
	local synthData = synthData
	local phase = synthData[1]

	local note = data[0]/sampleRate

	phase=phase+note
	phase=phase%1

	synthData[1] = phase

	local out=phase

	if(out > data[3]/2)then
		out=1
	else
		out=-1
	end

	return out,out
end