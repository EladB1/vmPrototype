#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "vm.h"
#include "frame.h"
#include "dataconstant.h"
#include "exitcode.h"

void print(DataConstant data, bool newLine);
void printerr(DataConstant data, bool terminates, int exitCode);
void sleep_(DataConstant seconds);
char* getType(DataConstant data);

char* at(char* str, int index, ExitCode* vmState);
bool startsWith_(char* string, char* prefix);
bool endsWith(char* string, char* suffix);
char* reverse(char* string);
bool contains(char* str, char* subStr);
char* replace(char* string, char* old, char* new, bool multiple);
char* slice(char* string, int start, int end, ExitCode* vmState);
DataConstant splitString(char* string, char* delim, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose);

bool fileExists(char* filePath);
void createFile(char* filePath, ExitCode* vmState);
DataConstant readFile(char* filePath, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose);
void writeToFile(char* filePath, char* content, char* mode, ExitCode* vmState);
void renameFile(char* filePath, char* newFilePath, ExitCode* vmState);
void deleteFile(char* filePath, ExitCode* vmState);

void reverseArr(DataConstant array);
DataConstant sliceArr(DataConstant array, int start, int end, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose);
bool arrayContains(DataConstant array, DataConstant element);
int indexOf(DataConstant array, DataConstant element);
char* join(DataConstant array, char* delim);
void sort(DataConstant array);
void removeByIndex(DataConstant* array, int index, ExitCode* vmState);
void append(DataConstant* array, DataConstant elem, ExitCode* vmState);
void prepend(DataConstant* array, DataConstant elem, ExitCode* vmState);
void insert(DataConstant* array, DataConstant elem, int index, ExitCode* vmState);

#endif