#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LINE_LEN 1000

#define MAX_LINES_NUMBER_TO_PRINT 100


#define issortable(c) ((c) != '.' && (c) != ',' && (c) != ';' && (c) != ':' && (c) != '!' && (c) != '?')


/*-------------------------------------------------- Error Handling --------------------------------------------------*/

#define errprintf(...) fprintf(stderr, __VA_ARGS__)

#define stopRunIf(isErr, codeToRunOnErr) \
    do {                                 \
        bool _isErr = isErr;             \
        if (_isErr) {                    \
            {codeToRunOnErr}             \
            return -1;                   \
        }                                \
    } while(false)

#define stopRunAndPrintErrIf(isErr, codeToRunOnErr, ...) \
    do {                                                 \
        bool _isErr = isErr;                             \
        if (_isErr) {                                    \
            errprintf(__VA_ARGS__);                      \
            {codeToRunOnErr}                             \
            return -1;                                   \
        }                                                \
    } while(false)


#define checkArgIsNotNull(arg) stopRunAndPrintErrIf(arg == NULL, , #arg" should not be equal to NULL.\n")


/*------------------------------------------------------ Vector ------------------------------------------------------*/

typedef char *VectorItem;

typedef struct {
    VectorItem *items;
    size_t cap;
    size_t size;
} Vector;


int Vector_init(Vector *vector, size_t initCapacity) {
    checkArgIsNotNull(vector);

    stopRunAndPrintErrIf(initCapacity == 0, , "Vector capacity should be positive.\n");

    vector->items = malloc(initCapacity * sizeof(VectorItem));
    stopRunAndPrintErrIf(vector->items == NULL, , "Memory allocation error.\n");

    vector->cap = initCapacity;
    vector->size = 0;

    return 0;
}

int Vector_push(Vector *vector, VectorItem item) {
    checkArgIsNotNull(vector);

    if (vector->size == vector->cap) {
        vector->cap *= 2;

        void *temp = realloc(vector->items, vector->cap * sizeof(VectorItem));
        stopRunAndPrintErrIf(temp == NULL, , "Memory allocation error.\n");
        vector->items = temp;
    }

    vector->items[vector->size++] = item;

    return 0;
}

int Vector_erase(Vector *vector) {
    checkArgIsNotNull(vector);

    free(vector->items);
    vector->items = NULL;

    return 0;
}


/*-------------------------------------------------------- App -------------------------------------------------------*/


int getInputFilename(int argc, char **argv, const char **pFilename) {
    checkArgIsNotNull(argv);
    checkArgIsNotNull(pFilename);

    stopRunAndPrintErrIf(argc != 2, , "Wrong number of arguments given.\n");

    *pFilename = argv[1];

    return 0;
}

int readAndParseInputFile(const char *inputFilename, Vector *lines) {
    checkArgIsNotNull(inputFilename);
    checkArgIsNotNull(lines);

    FILE *inputFile = fopen(inputFilename, "r");
    stopRunAndPrintErrIf(inputFile == NULL, , "Cannot open the inputFile.\n");

    stopRunIf(Vector_init(lines, 16), { fclose(inputFile); });


    char buf[MAX_LINE_LEN];
    while (fgets(buf, MAX_LINE_LEN, inputFile)) {
        size_t bufSize = strlen(buf);

        size_t lineSize = 0;
        for (size_t i = 0; i < bufSize; ++i) {
            char c = buf[i];
            if (issortable(c)) lineSize++;
        }

        char *line = malloc(lineSize + 1);
        stopRunAndPrintErrIf(line == NULL, {
            Vector_erase(lines);
            fclose(inputFile);
        }, "Memory allocation error.\n");

        line[lineSize] = '\0';
        for (long long i = (long long) bufSize - 1; i >= 0; --i) {
            char c = buf[i];
            if (issortable(c)) line[--lineSize] = buf[i];
        }


        stopRunIf(Vector_push(lines, line), {
            Vector_erase(lines);
            fclose(inputFile);
        });
    }

    stopRunAndPrintErrIf(ferror(inputFile), {
        Vector_erase(lines);
        fclose(inputFile);
    }, "Cannot read from the file.\n");

    stopRunAndPrintErrIf(fclose(inputFile), , "Cannot close the inputFile.\n");

    return 0;
}

int printFirstLines(Vector lines) {
    size_t size = MAX_LINES_NUMBER_TO_PRINT < lines.size ? MAX_LINES_NUMBER_TO_PRINT : lines.size;
    for (size_t i = 0; i < size; ++i) {
        stopRunAndPrintErrIf(printf("%s", lines.items[i]) < 0, , "Cannot print to stdout.\n");
    }

    return 0;
}


int linesCmp(const void *a, const void *b) { return strcmp(*(char *const *) b, *(char *const *) a); }

int main(int argc, char *argv[]) {
    const char *inputFilename;
    if (getInputFilename(argc, argv, &inputFilename)) return EXIT_FAILURE;

    Vector lines;
    if (readAndParseInputFile(inputFilename, &lines)) return EXIT_FAILURE;

    qsort(lines.items, lines.size, sizeof(VectorItem), linesCmp);

    if (printFirstLines(lines)) return EXIT_FAILURE;

    if (Vector_erase(&lines)) return EXIT_FAILURE;


    return EXIT_SUCCESS;
}
