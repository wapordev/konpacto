#ifndef WWPAGE_H_
#define WWPAGE_H_
/* ^^ these are the include guards */

#ifndef PATH_MAX
#include <linux/limits.h>
#endif

#include <stdint.h>

extern UIPage projectPage;

extern UIPage arrangePage;

extern UIPage trackPage;

extern UIPage operatorPage;

extern UIPage saveFilePage;

extern UIPage loadFilePage;

extern uint8_t instrumentIndex;

extern uint8_t selectedTrack;

extern char selectedFile[PATH_MAX];
extern char selectedFileName[PATH_MAX];
extern char fileBrowserPath[PATH_MAX];

void ReloadFileBrowse(int offset);

#endif
