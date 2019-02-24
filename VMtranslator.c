#include "includes.h"
#include "helper.h"
#include "parser.h"
#include "codeWriter.h"

int main(int argc, char** argv) {
    checkAndLoadArgs(argc, argv);
    initializeParser(input);
    initializeCodeWriter(outputFileName);

    while (hasMoreCommands()) {
        if (advance())
            translate();
        if (!hasMoreCommands())
            nextFile();
    }

    saveChanges();    

    return EXIT_SUCCESS;
}