#include "helper.h"
#include "parser.h"
#include "codeWriter.h"

void checkAndLoadArgs(const int argc, char **argv) {
    if (argc < 2) {
        printf("You need to specify an input vm file\n");
        exit(EXIT_FAILURE);
    } else if (strlen(argv[1]) > FILE_NAMES_MAX_LENGTH) {
        printf("The input file name is too long");
        exit(EXIT_FAILURE);
    } else if (argc >= 3) {
        if (strlen(argv[2]) > FILE_NAMES_MAX_LENGTH) {
            printf("The output file name is too long");
            exit(EXIT_FAILURE);
        }
    }

    strcpy(input, argv[1]);
    if (argc < 3) {
        int count = 0;
        int slashes = 0;

        while (input[count] != '\0') {
            if (input[count] == '\\' || input[count] == '/')
                slashes++;
            count++;
        }

        count = 0;
        while (slashes > 0 && input[count] != '\0') {
            if (input[count] == '\\' || input[count] == '/')
                slashes--;
            count++;
        }

        int start = count;
        while (input[count] != '.' && input[count] != '\0' && count < FILE_NAMES_MAX_LENGTH - 5) {
            outputFileName[count - start] = input[count];
            count++;
        }
        strcat(outputFileName, ".asm");
    } else
        strcpy(outputFileName, argv[2]);
}

int isNaNChar(const char c) {
    return c < '0' || c > '9';
}

int parseInt(char s[]) {
    int wrong = 0;
    int i = 0;
    int value = 0;

    while (s[i] != '\0' && !wrong) {
        if (!isNaNChar(s[i]))
            value = (value + (s[i] - '0'));
        else wrong = 1;
        i++;
        if (s[i] != '\0')
            value *= 10;
    }

    if (wrong) return -1;
    else return value;
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

void saveChanges() {
    fclose(fin);
    close();
}
