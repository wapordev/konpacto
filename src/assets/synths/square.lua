print(-.9)

_defaultParams = {{"silly",2}}

phase = 0

function _audioFrame(data)

	if data[1]==0 then return 0,0 end

	phase=phase+data[2]/data[3]
	phase=phase%1

	local out=phase*2-1

	if out>0 then
		out=1
	else
		out=-1
	end

	return out,out
end