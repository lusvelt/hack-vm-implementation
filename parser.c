#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fin;
char currentCommand[COMMAND_MAX_LENGTH];
int currentLine;

struct command_t command;

void initializeParser(const char* inputFileName) {
    fin = fopen(inputFileName, "r");

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

    if (currentCommand[count - 1] == ' ')
        count--;

    currentCommand[count] = '\0';

    parseAndCheckCommand();

    return 1;
}

char** splitBySpaces(char s[]) {
    char *p = currentCommand;
    char** temp = (char **) malloc(3 * sizeof(char *));
    int count = 0, index;
    do {
        index = 0;
        temp[count] = (char *) malloc(sizeof(char[SYMBOL_MAX_LENGTH]));
        do {
            temp[count][index] = *p;
            index++;
            p++;
        } while (*p != '\0' && *p != ' ');
        temp[count][index] = '\0';
        if (*p == ' ') {
            p++;
            count++;
        }
    } while (*p != '\0');
    count++;
    while (count < 3) {
        temp[count] = (char *) malloc(sizeof(char));
        strcpy(temp[count], "");
        count++;
    }
    return temp;
}

void checkArguments(int expected) {
    int argc = 0;
    for (int i = 1; i < 3; i++)
        if (strcmp(command.args[i], "") != 0)
            argc++;
    if (argc != expected) {
        char* error = (char*) malloc(sizeof(char[256]));
        char* tmp = (char*) malloc(sizeof(char[2]));
        tmp[1] = '\0';

        strcpy(error, "Expected ");
        tmp[0] = (char) ((int) '0' + expected);
        strcat(error, tmp);
        strcat(error, " arguments but got ");
        tmp[0] = (char) ((int) '0' + argc);
        strcat(error, tmp);
        throwError(error);
    }
    if (strlen(command.args[1]) > SYMBOL_MAX_LENGTH)
        throwError("Symbol contains too many characters");
}

enum memorySegment getSegment(char s[]) {
    if (strcmp(s, "argument") == 0) return M_ARGUMENT;
    else if (strcmp(s, "local") == 0) return M_LOCAL;
    else if (strcmp(s, "static") == 0) return M_STATIC;
    else if (strcmp(s, "constant") == 0) return M_CONSTANT;
    else if (strcmp(s, "this") == 0) return M_THIS;
    else if (strcmp(s, "that") == 0) return M_THAT;
    else if (strcmp(s, "pointer") == 0) return M_POINTER;
    else if (strcmp(s, "temp") == 0) return M_TEMP;
    else throwError("Unknown memory segment");
}

void checkIndex(int index, int max) {
    if (index < 0 || index >= max)
        throwError("Invalid index value");
}

int getIndex(char s[]) {
    int index = parseInt(s);

    if (index < 0)
        throwError("Invalid index value");
    else {
        if (command.segment == M_ARGUMENT) checkIndex(index, 65536);
        if (command.segment == M_LOCAL) checkIndex(index, 65536);
        if (command.segment == M_STATIC) checkIndex(index, 65536);
        if (command.segment == M_CONSTANT) checkIndex(index, 32768);
        if (command.segment == M_THIS) checkIndex(index, 65536);
        if (command.segment == M_THAT) checkIndex(index, 65536);
        if (command.segment == M_POINTER) checkIndex(index, 65536);
        if (command.segment == M_TEMP) checkIndex(index, 65536);
        
        return index;
    }
}

int getVars(char s[]) {
    int vars = parseInt(s);

    if (vars < 0 || vars >= 65536)
        throwError("Invalid local vars number");
    else return vars;
}

void parseAndCheckCommand() {
    command.args = splitBySpaces(currentCommand);

    if (
        strcmp(command.args[0], "add")  == 0 ||
        strcmp(command.args[0], "sub")  == 0 ||
        strcmp(command.args[0], "neg")  == 0 ||
        strcmp(command.args[0], "eq")   == 0 ||
        strcmp(command.args[0], "gt")   == 0 ||
        strcmp(command.args[0], "lt")   == 0 ||
        strcmp(command.args[0], "and")  == 0 ||
        strcmp(command.args[0], "or")   == 0 ||
        strcmp(command.args[0], "not")  == 0
    ) {
        checkArguments(0);
        command.type = C_ARITHMETIC;
    } else if (strcmp(command.args[0], "push") == 0) {
        checkArguments(2);
        command.type = C_PUSH;
        command.segment = getSegment(command.args[1]);
        command.index = getIndex(command.args[2]);
    } else if (strcmp(command.args[0], "pop") == 0) {
        checkArguments(2);
        command.type = C_POP;
        command.segment = getSegment(command.args[1]);
        command.index = getIndex(command.args[2]);
    } else if (strcmp(command.args[0], "label") == 0) {
        checkArguments(1);
        command.type = C_LABEL;
        strcpy(command.label, command.args[1]);
    } else if (strcmp(command.args[0], "goto") == 0) {
        checkArguments(1);
        command.type = C_GOTO;
        strcpy(command.label, command.args[1]);
    } else if (strcmp(command.args[0], "if-goto") == 0) {
        checkArguments(1);
        command.type = C_IF;
        strcpy(command.label, command.args[1]);
    } else if (strcmp(command.args[0], "function") == 0) {
        checkArguments(2);
        command.type = C_FUNCTION;
        strcpy(command.label, command.args[1]);
        command.vars = getVars(command.args[2]);
    } else if (strcmp(command.args[0], "return") == 0) {
        checkArguments(0);
        command.type = C_RETURN;
    } else if (strcmp(command.args[0], "call") == 0) {
        checkArguments(2);
        command.type = C_CALL;
        strcpy(command.label, command.args[1]);
        command.vars = getVars(command.args[2]);
    } else
        throwError("Command not recognized");
}

char* arg1() {
    return command.args[1];
}

char* arg2() {
    return command.args[2];
}
