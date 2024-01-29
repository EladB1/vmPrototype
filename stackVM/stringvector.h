#define DEFAULT_VECTOR_MAX 256
#define MAX(a, b) (a >= b ? a : b)

typedef struct {
    char** strings;
    int length;
    int capacity;
} StringVector;

StringVector* createStringVector();
void freeStringVector(StringVector* sv);
void addString(StringVector* sv, char* string);
StringVector* concat(StringVector* sv1, StringVector* sv2);
void printStringVector(StringVector* sv);
StringVector* split(char* line, char* delim);
void trim(StringVector* sv);