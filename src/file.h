#ifndef WWFILE_H_
#define WWFILE_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */

char* IntToChar(int BMPstring[], char* out, int length);

char* CharToBMP(char* string, bool filter);

void ListPath();

void FreePath();

#endif
