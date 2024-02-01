typedef struct {
    char** strings;
    int length;
    int capacity;
} StringVector;

StringVector* createStringVector();
void freeStringVector(StringVector*);
char* getFromSV(StringVector*, int);
void addString(StringVector*, char*);
StringVector* concat(StringVector*, StringVector*);
void printStringVector(StringVector*);
StringVector* split(char*, char*);
void trimSV(StringVector*);