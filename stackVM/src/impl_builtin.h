#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "dataconstant.h"
#include "exitcode.h"

void print(DataConstant data, bool newLine);
void printerr(DataConstant data, bool terminates, int exitCode);
void sleep_(DataConstant seconds);
char* getType(DataConstant data, DataConstant* globals);

char* at(char* str, int index, ExitCode* vmState);
bool startsWith_(char* string, char* prefix);
bool endsWith(char* string, char* suffix);
char* reverse(char* string);
bool contains(char* str, char* subStr);
char* replace(char* string, char* old, char* new, bool multiple);
char* slice(char* string, int start, int end, ExitCode* vmState);

bool fileExists(char* filePath);
void createFile(char* filePath, ExitCode* vmState);
DataConstant readFile(char* filePath, int* globCount, DataConstant** globals, ExitCode* vmState);
void writeToFile(char* filePath, char* content, char* mode, ExitCode* vmState);
void renameFile(char* filePath, char* newFilePath, ExitCode* vmState);
void deleteFile(char* filePath, ExitCode* vmState);

void reverseArr(DataConstant array, DataConstant** globals);
DataConstant sliceArr(DataConstant array, int start, int end, int* globCount, DataConstant** globals, ExitCode* vmState);
bool arrayContains(DataConstant array, DataConstant element, DataConstant* globals);
int indexOf(DataConstant array, DataConstant element, DataConstant* globals);
char* join(DataConstant array, char* delim, DataConstant* globals);
void sort(DataConstant array, DataConstant* globals);
void removeByIndex(DataConstant* array, int index, DataConstant** globals, ExitCode* vmState);
void append(DataConstant* array, DataConstant elem, DataConstant** globals, ExitCode* vmState);
void prepend(DataConstant* array, DataConstant elem, DataConstant** globals, ExitCode* vmState);
void insert(DataConstant* array, DataConstant elem, int index, DataConstant** globals, ExitCode* vmState);

#endif