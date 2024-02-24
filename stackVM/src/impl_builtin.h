#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "dataconstant.h"

void print(DataConstant data, DataConstant* globals, bool newLine);
void printerr(DataConstant data, bool terminates, int exitCode);
void sleep_(DataConstant seconds);
char* getType(DataConstant data, DataConstant* globals);

char* at(char* str, int index);
bool startsWith_(char* string, char* prefix);
bool endsWith(char* string, char* suffix);
char* reverse(char* string);
bool contains(char* str, char* subStr);
char* replace(char* string, char* old, char* new, bool multiple);
char* slice(char* string, int start, int end);

bool fileExists(char* filePath);
void createFile(char* filePath);
DataConstant readFile(char* filePath, int* globCount, DataConstant** globals);
void writeToFile(char* filePath, char* content, char* mode);
void renameFile(char* filePath, char* newFilePath);
void deleteFile(char* filePath);

void reverseArr(DataConstant array, DataConstant** globals);
DataConstant sliceArr(DataConstant array, int start, int end, int* globCount, DataConstant** globals);
bool arrayContains(DataConstant array, DataConstant element, DataConstant* globals);
int indexOf(DataConstant array, DataConstant element, DataConstant* globals);
char* join(DataConstant array, char* delim, DataConstant* globals);
void sort(DataConstant array, DataConstant* globals);
void removeByIndex(DataConstant* array, int index, DataConstant** globals);
void append(DataConstant* array, DataConstant elem, DataConstant** globals);
void prepend(DataConstant* array, DataConstant elem, DataConstant** globals);
void insert(DataConstant* array, DataConstant elem, int index, DataConstant** globals);

#endif