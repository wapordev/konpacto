_defaultParams = {
	{"in gain",30},
	{"length",255},
	{"pitch mult",0},
	{"pulsewidth",200},
	{"percent",255}
}

function _init()
	return{{[0]=0,0},0,0}
end

function _audioFrame(synthData)
	local length = (floor(C.konGet(4)*255))*2+1

	local buffer = synthData[1]
	local index = synthData[2]%(length+1)

	--buffer relengther
	for i=#buffer+1,length do
		buffer[i]=0
	end

	
	local phase = synthData[3]

	--note multiplier
	local note = (C.konGet(0)* (C.konGet(5)*7.96875)  )/sampleRate

	phase=(phase+note)%1


	local sqr=phase*2-1

	--generator. window gated saw and sin wave
	if(sqr > C.konGet(6)*2-1)then
		--NOTE this isnt actually a full sin!!! that would be tau
		--i made a mistake. tau will give interesting results but ruin others
		--maybe a parameter?
		sqr=sin(phase*pi)
	end

	--percentage
	for i=0,floor(length*C.konGet(7)),2 do
		
		--salting, so that small percentages do not ring.
		local i=(i+index*2)%(length+1)

		local target = buffer[i]
		local real = buffer[i+1]

		local dist = abs(real-target)
		if abs(target-sqr) < dist then
			buffer[i+1]=sqr
		end
	end
	
	local out=buffer[index+1]

	local current = C.konGet(32)

	buffer[index] = current*(C.konGet(3)*32)--current+buffer[nextIndex]*C.konGet(4)

	synthData[2] = index+2
	synthData[3] = phase

	C.konOut(out,out)
end