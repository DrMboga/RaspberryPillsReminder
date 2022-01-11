#include <stdio.h>
#include "file-access.h"

FILE* OpenFileForRead(const char* fileName)
{
    return fopen(fileName, "r");
}

FILE* OpenFileForRewrite(const char* fileName)
{
    return fopen(fileName, "w+");
}

FILE* OpenFileForAppend(const char* fileName)
{
    return fopen(fileName, "a+");
}

int CheckError(FILE* filePointer)
{
    int errNumber = ferror(filePointer);
    if(errNumber != 0)
    {
        printf("File error number %d", errNumber);
        perror("Error reading or writing to file\n");
        return -1;
    }
    return 0;
}

int CloseFile(FILE* filePointer)
{
    if(fclose(filePointer) == EOF)
    {
        perror("Error closing file\n");
        return -1;
    }
    return 0;
}