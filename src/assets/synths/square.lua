_defaultParams = {{"tilly",2}}

function _init()
	return{0}
end

function _audioFrame(synthData,on,note)
	local synthData = synthData
	local phase = synthData[1]

	if on==0 then return 0,0 end

	phase=phase+note/44100
	phase=phase%1

	synthData[1] = phase

	local out=phase*2-1

	if out>0 then
		out=1
	else
		out=-1
	end

	return out,out
end