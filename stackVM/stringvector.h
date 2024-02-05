typedef struct {
    char** strings;
    int length;
    int capacity;
} StringVector;

StringVector* createStringVector();
void freeStringVector(StringVector* sv);
char* getFromSV(StringVector* sv, int);
void addString(StringVector* sv, char*);
StringVector* concat(StringVector* sv1, StringVector* sv2);
void printStringVector(StringVector* sv);
StringVector* split(char* line, char* delim);
void trimSV(StringVector* sv);