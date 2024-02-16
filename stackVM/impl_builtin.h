#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "dataconstant.h"

void sleep_(DataConstant seconds);
char* at(char* str, int index);
bool startsWith_(char* string, char* prefix);
bool endsWith(char* string, char* suffix);
char* reverse(char* string);
void reverseArr(DataConstant array, DataConstant** globals);
bool fileExists(char* filePath);
void createFile(char* filePath);
DataConstant readFile(char* filePath, int* globCount, DataConstant** globals);
void writeToFile(char* filePath, char* content, char* mode);
void renameFile(char* filePath, char* newFilePath);
void deleteFile(char* filePath);
void print(DataConstant data, DataConstant* globals, bool newLine);
void printerr(DataConstant data, bool terminates, int exitCode);
char* slice(char* string, int start, int end);
DataConstant sliceArr(DataConstant array, int start, int end, int* globCount, DataConstant** globals);
bool contains(char* str, char* subStr);
bool arrayContains(DataConstant array, DataConstant element, DataConstant* globals);
int indexOf(DataConstant array, DataConstant element, DataConstant* globals);
char* getType(DataConstant data, DataConstant* globals);
char* join(DataConstant array, char* delim, DataConstant* globals);
void sort(DataConstant array, DataConstant* globals);
void removeByIndex(DataConstant* array, int index, DataConstant** globals);

#endif