_defaultParams = {{"silly",2}}

function _init()
	return{0}
end

function _audioFrame(synthData)
	local synthData = synthData
	local phase = synthData[1]

	local note = C.konGet(0)

	phase=phase+note
	phase=phase%1

	synthData[1] = phase

	local out=phase*2-1

	C.konOut(out,out)
end