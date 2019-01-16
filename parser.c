#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fin, *fout;
char currentCommand[COMMAND_MAX_LENGTH];
int currentLine;

struct command_t command;

void initializeParser(const char* inputFileName, const char* outputFileName) {
    fin = fopen(inputFileName, "r");
    fout = fopen(outputFileName, "w");

    if (fin == NULL) {
        printf("The specified input file was not found\n");
        exit(EXIT_FAILURE);
    }

    strcpy(currentCommand, "");
    currentLine = 0;
}

int hasMoreCommands() {
    return !feof(fin);
}

void scanTrimSpaces(char* c, int* comment, const int alreadyStarted) {
    char p = *c;
    do {
        do fscanf(fin, "%c", c);
        while (*c == ' ' && (!alreadyStarted || p == ' ') && hasMoreCommands());

        if (*c == '\n') {
            currentLine++;
            *comment = 0;
        }
    } while (!alreadyStarted && *c == '\n' && hasMoreCommands());
}

void throwError(const char* error) {
    printf("Fatal error at line %d: %s\n", currentLine, error);
    exit(EXIT_FAILURE);
}

int advance() {
    char c;
    int comment, count, wrong;
    
    comment = 0;
    count = 0;

    do {
        scanTrimSpaces(&c, &comment, count > 0);

        if (c == '/') comment++;
        else if (hasMoreCommands()) {
            if (comment == 1) comment = 0;
            if (c != '\n' && !comment)
                currentCommand[count++] = c;
        }
    } while (c != '\n' && count < COMMAND_MAX_LENGTH && hasMoreCommands());

    if (!hasMoreCommands() && count == 0)
        return 0;

    if (count == COMMAND_MAX_LENGTH)
        throwError("The instruction contains too many characters");

    currentCommand[count] = '\0';

    parseCommand();
    checkCommand();

    return 1;
}

void parseCommand() {
    
}

void checkCommand() {

}

char* arg1() {

}

char* arg2() {

}

void saveChanges() {
    fclose(fin);
    fclose(fout);
}