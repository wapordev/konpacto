_defaultParams = {{"silly",2}}

function _init()
	return{0}
end

function _audioFrame(on,note,synthData)
	local synthData = synthData
	local phase = synthData[1]

	if on==0 then return 0,0 end

	phase=phase+note/44100
	phase=phase%1

	synthData[1] = phase

	local out=phase*2-1

	return out,out
end