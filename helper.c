#include "helper.h"
#include "parser.h"

void checkAndLoadArgs(const int argc, char **argv) {
    if (argc < 2) {
        printf("You need to specify an input assembly file\n");
        exit(EXIT_FAILURE);
    } else if (strlen(argv[1]) > FILE_NAMES_MAX_LENGTH) {
        printf("The input file name is too long");
        exit(EXIT_FAILURE);
    } else {
        if (strlen(argv[2]) > FILE_NAMES_MAX_LENGTH) {
            printf("The output file name is too long");
            exit(EXIT_FAILURE);
        }
    }

    strcpy(inputFileName, argv[1]);
    if (argc < 3) {
        int count = 0;
        int slashes = 0;

        while (inputFileName[count] != '\0') {
            if (inputFileName[count] == '\\' || inputFileName[count] == '/')
                slashes++;
            count++;
        }

        count = 0;
        while (slashes > 0 && inputFileName[count] != '\0') {
            if (inputFileName[count] == '\\' || inputFileName[count] == '/')
                slashes--;
            count++;
        }

        int start = count;
        while (inputFileName[count] != '.' && inputFileName[count] != '\0' && count < FILE_NAMES_MAX_LENGTH - 5) {
            outputFileName[count - start] = inputFileName[count];
            count++;
        }
        strcpy(outputFileName + count - start, ".asm");
    } else
        strcpy(outputFileName, argv[2]);
}

int isNaNChar(const char c) {
    return c < '0' || c > '9';
}

int strsrch(const char *s, const char c) {
    int count = 0, found = 0;

    while (s[count] != '\0' && !found) {
        if (s[count] == c)
            found = 1;
        else
            count++;
    }

    if (found)
        return count;
    else
        return -1;
}
