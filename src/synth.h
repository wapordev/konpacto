#ifndef WWSYNTH_H_
#define WWSYNTH_H_

typedef struct AudioState {
	int phase;
	int dummy;
}AudioState;

extern AudioState* audioState;

#endif
