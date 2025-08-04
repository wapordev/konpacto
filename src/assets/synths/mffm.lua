_defaultParams = {
	{"O1 self-mod",0},
	{"O1 feedback",0},
	{"O2 volume",0},
	{"O2 feedback",0},
}

function _init()
	return{0,0,0}
end

function _audioFrame(synthData,data)
	local synthData = synthData
	local phase,o1,o2 = synthData[1],synthData[2],synthData[3]

	local note = data[0]/sampleRate
	local selfmod,feedback1,volume2,feedback2 = data[3]*8,data[4],data[5],data[6]

	phase=phase+note

	


	o2 = sin((phase+o2*feedback2)*pi*2)*volume2

	local self = sin(phase*pi*2)*selfmod

	o1 = sin((phase+o1*feedback1+self+o2)*pi*2)

	local out = o1
	

	synthData[1],synthData[2],synthData[3] = phase,o1,o2

	return out,out
end