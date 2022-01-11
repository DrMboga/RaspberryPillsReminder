#include <stdio.h>

/*
* Opens file for read, returns null if file doesn't exist
*/
FILE* OpenFileForRead(const char* fileName);

/*
* Opens file for rewrite, clears file if it exists
*/
FILE* OpenFileForRewrite(const char* fileName);

/*
* Opens file for append
*/
FILE* OpenFileForAppend(const char* fileName);

/*
* Checks and I/O error and prints it if exists
*/
int CheckError(FILE* filePointer);

/*
* Frees file pointer
*/
int CloseFile(FILE* filePointer);