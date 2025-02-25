_defaultParams = {{"silly",2}}

function _init()
	return{0}
end

function _audioFrame(stepData,synthData)
	local synthData = synthData
	local phase = synthData[1]

	if stepData[1]==0 then return 0,0 end

	phase=phase+stepData[2]/stepData[3]
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