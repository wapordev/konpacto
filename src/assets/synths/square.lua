_defaultParams = {
	{"pulse width",255},
}

function _init()
	return{0}
end

function _audioFrame(synthData)
	local phase = synthData[1]

	local note = C.konGet(0)/sampleRate

	phase=phase+note
	phase=phase%1

	synthData[1] = phase

	local out=phase

	if(out > C.konGet(3)/2)then
		out=10000000
	else
		out=-10000000
	end

	C.konOut(out,out)
end