_defaultParams = {
	{"length",255},
	{"feedback",80},
}

function _init()
	return{{[0]=0,0},0}
end

function _audioFrame(synthData)
	local length = floor(C.konGet(3)*100000)

	local buffer = synthData[1]
	local index = synthData[2]

	for i=#buffer+1,length do
		buffer[i]=0
	end

	local current = C.konGet(32)

	local nextIndex = (index+1)%length

	buffer[index] = current+buffer[nextIndex]*C.konGet(4)

	index=nextIndex

	local out=buffer[index]

	synthData[2] = index

	C.konOut(out,out)
end