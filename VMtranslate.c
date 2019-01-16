#include "includes.h"
#include "helper.h"
#include "parser.h"

int main(int argc, char** argv) {
    checkAndLoadArgs(argc, argv);
    initializeParser(inputFileName, outputFileName);

    while (hasMoreCommands()) {
        advance();
        fprintf(fout, "%s\n", currentCommand);
    }

    saveChanges();    

    return EXIT_SUCCESS;
}