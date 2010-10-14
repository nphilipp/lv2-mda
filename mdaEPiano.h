#ifndef MDA_EPIANO_H
#define MDA_EPIANO_H
//See associated .cpp file for copyright and other info

class mdaEPianoProgram
{
	friend class mdaEPiano;
	private:
	float param[NPARAMS];
	char  name[24];
};

#endif
