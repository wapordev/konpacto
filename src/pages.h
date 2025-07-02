#ifndef WWPAGE_H_
#define WWPAGE_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */

extern UIPage projectPage;

extern UIPage arrangePage;

extern UIPage trackPage;

extern UIPage operatorPage;

extern UIPage saveFilePage;

extern UIPage loadFilePage;

extern char selectedFile[PATH_MAX];
extern char selectedFileName[PATH_MAX];
extern char fileBrowserPath[PATH_MAX];

void ReloadFileBrowse(int offset);

#endif
