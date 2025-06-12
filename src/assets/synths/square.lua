_defaultParams = {
	{"pulse width",255},
	{"junk",235},
}

function _init()
	return{0}
end

local phase = 0

function _audioFrame(synthData)

	

	local note = C.konGet(0)/sampleRate

	phase=phase+note
	phase=phase%1

	local out=phase

	if(out > C.konGet(3)/510)then
		out=1
	else
		out=-1
	end

	C.konOut(out,out)
end