_defaultParams = {
	{"a",2},
	{"bee",2},
	{"c",2},
	{"d",2},
	{"e",2},
	{"readfromlua",2},
	{"g",2},
	{"h",2},
	{"i",2},
	{"j",2},
	{"k",2},
	{"l",2},
	{"m",2},
	{"n",2},
	{"o",2},
	{"hihi",2},
}

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