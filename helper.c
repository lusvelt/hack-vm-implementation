#include "helper.h"
#include "parser.h"

void checkAndLoadArgs(const int argc, char **argv) {
    if (argc < 2) {
        printf("You need to specify an input assembly file\n");
        exit(EXIT_FAILURE);
    } else if (argc > 3) {
        printf("Too many arguments. Specify only input and output file names.");
        exit(EXIT_FAILURE);
    } else if (strlen(argv[1]) > FILE_NAMES_MAX_LENGTH) {
        printf("The input file name is too long");
        exit(EXIT_FAILURE);
    } else if (argc == 3) {
        if (strlen(argv[2]) > FILE_NAMES_MAX_LENGTH) {
            printf("The output file name is too long");
            exit(EXIT_FAILURE);
        }
    }

    strcpy(inputFileName, argv[1]);
    if (argc < 3) {
        int cont = 0;
        int slashes = 0;

        while (inputFileName[cont] != '\0') {
            if (inputFileName[cont] == '\\' || inputFileName[cont] == '/')
                slashes++;
            cont++;
        }

        cont = 0;
        while (slashes > 0 && inputFileName[cont] != '\0') {
            if (inputFileName[cont] == '\\' || inputFileName[cont] == '/')
                slashes--;
            cont++;
        }

        int start = cont;
        while (inputFileName[cont] != '.' && inputFileName[cont] != '\0' && cont < FILE_NAMES_MAX_LENGTH - 5) {
            outputFileName[cont - start] = inputFileName[cont];
            cont++;
        }
        strcpy(outputFileName + cont - start, ".asm");
    } else
        strcpy(outputFileName, argv[2]);
}

int isNaNChar(const char c) {
    return c < '0' || c > '9';
}

int strsrch(const char *s, const char c) {
    int cont = 0, found = 0;

    while (s[cont] != '\0' && !found) {
        if (s[cont] == c)
            found = 1;
        else
            cont++;
    }

    if (found)
        return cont;
    else
        return -1;
}
