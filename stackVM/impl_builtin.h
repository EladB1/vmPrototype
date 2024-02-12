#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "dataconstant.h"

void sleep_(DataConstant seconds);
char* at(char* str, int index);
bool startsWith_(char* string, char* prefix);
bool endsWith(char* string, char* suffix);
char* reverse(char* string);
bool fileExists(char* filePath);
void createFile(char* filePath);
char** readFile(char* filePath);
void writeToFile(char* filePath, char* content, char* mode);
void renameFile(char* filePath, char* newFilePath);
void deleteFile(char* filePath);
void print(DataConstant data, DataConstant* globals, bool newLine);
void printerr(DataConstant data, bool terminates, int exitCode);
char* slice(char* string, int start, int end);
bool contains(char* str, char* subStr);
bool arrayContains(DataConstant array, DataConstant element, DataConstant* globals);

#endif