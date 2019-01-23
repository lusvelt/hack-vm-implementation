#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];
char currentFunction[SYMBOL_MAX_LENGTH];
int comparisonsCount = 0;
int callsCount = 0;

void writeInit() {
    fprintf(fout, "@256\nD=A\n@SP\nM=D\n");
}

void initializeCodeWriter(const char* outputFileName) {
    fout = fopen(outputFileName, "w");
    writeInit();
}

void setFileName(const char* fileName) {

}

void prepareOperation() {
    fprintf(fout, "@SP\nA=M\nA=A-1\n");
}

void prepareSecondOperand() {
    fprintf(fout, "D=M\nA=A-1\n");
}

void updateAfterBinaryOperation() {
    fprintf(fout, "D=A+1\n@SP\nM=D\n");
}

void operate(const char instructions[], int isBinary) {
    prepareOperation();

    if (isBinary)
        prepareSecondOperand();

    fprintf(fout, instructions);
    
    if (isBinary)
        updateAfterBinaryOperation();
}

void compare(const char jumpMnemonic[]) {
    operate("M=M-D\n", 1);
    prepareOperation();

    fprintf(fout, "D=M\n@START_CMP:%d\nD;%s\n", comparisonsCount, jumpMnemonic);
    fprintf(fout, "@ELSE_CMP:%d\n0;JMP\n", comparisonsCount);
    fprintf(fout, "(START_CMP:%d)\n@SP\nA=M\nA=A-1\nM=0\nM=!M\n", comparisonsCount);
    fprintf(fout, "@END_CMP:%d\n0;JMP\n", comparisonsCount);
    fprintf(fout, "(ELSE_CMP:%d)\n@SP\nA=M\nA=A-1\nM=0\n", comparisonsCount);
    fprintf(fout, "(END_CMP:%d)\n", comparisonsCount);

    comparisonsCount++;
}

void writeArithmetic(const char* command) {
    if (strcmp(command, "add") == 0)
        operate("M=D+M\n", 1);
    else if (strcmp(command, "sub") == 0)
        operate("M=M-D\n", 1);
    else if (strcmp(command, "neg") == 0)
        operate("M=-M\n", 0);
    else if (strcmp(command, "eq") == 0)
        compare("JEQ");
    else if (strcmp(command, "gt") == 0)
        compare("JGT");
    else if (strcmp(command, "lt") == 0)
        compare("JLT");
    else if (strcmp(command, "and") == 0)
        operate("M=D&M\n", 1);
    else if (strcmp(command, "or") == 0)
        operate("M=D|M\n", 1);
    else if (strcmp(command, "not") == 0)
        operate("M=!M\n", 0);        
}

void push(int index, char address[]) {
    fprintf(fout, "@%d\nD=A\n@%s\nA=D+M\nD=M\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", index, address);
}

void pushConstant(int constant) {
    fprintf(fout, "@%d\nD=A\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", constant);
}

void pushLabel(char label[]) {
    fprintf(fout, "@%s\nD=M\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", label, label);
}

void pop(int index, char address[]) {
    fprintf(fout, "@%d\nD=A\n@%s\nD=D+M\n@R5\nM=D\n@SP\nAM=M-1\nD=M\n@R5\nA=M\nM=D\n", index, address);
}

void writePushPop(enum commandType type, enum memorySegment segment, int index) {
    if (type == C_PUSH) {
        if (segment == M_CONSTANT)
            pushConstant(index);
        else if (segment == M_STATIC)
            fprintf(fout, "@static:%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", index);
        else if (segment == M_ARGUMENT)
            push(index, "ARG");
        else if (segment == M_LOCAL)
            push(index, "LCL");
    } else if (type == C_POP) {
        if (segment == M_CONSTANT)
            throwError("Cannot pop from segment 'constant'");
        else if (segment == M_STATIC)
            fprintf(fout, "@SP\nAM=M-1\nD=M\n@static:%d\nM=D\n", index);
        else if (segment == M_ARGUMENT)
            pop(index, "ARG");
        else if (segment == M_LOCAL)
            pop(index, "LCL");
    }
}

void writeLabel(char label[]) {
    fprintf(fout, "(%s$%s)\n", currentFunction, label, label);
}

void writeGoto(char label[]) {
    fprintf(fout, "@%s$%s\n0;JMP\n", currentFunction, label);
}

void writeIf(char label[]) {
    fprintf(fout, "@SP\nAM=M-1\nD=M\n@%s$%s\nD;JNE\n", currentFunction, label);
}

void writeCall(char functionName[], int numArgs) {
    fprintf(fout, "@return:%d\nD=A\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", callsCount);
    pushLabel("LCL");
    pushLabel("ARG");
    pushLabel("THIS");
    pushLabel("THAT");
    fprintf(fout, "@%d\nD=A\n@5\nD=D+A\n@SP\nD=M-D\n@ARG\nM=D\n", numArgs);
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n");
    fprintf(fout, "@%s\n0;JMP\n", functionName);
    fprintf(fout, "(return:%d)\n", callsCount);

    callsCount++;
}

void writeReturn() {
    fprintf(fout, "@LCL\nD=M\n@R6\nM=D\n");
    fprintf(fout, "@5\nD=A\n@R6\nA=M-D\nD=M\n@R7\nM=D\n");
    fprintf(fout, "@SP\nA=M-1\nD=M\n@ARG\nA=M\nM=D\n");
    fprintf(fout, "@ARG\nD=M+1\n@SP\nM=D\n");
    fprintf(fout, "@R6\nAM=M-1\nD=M\n@THAT\nM=D\n");
    fprintf(fout, "@R6\nAM=M-1\nD=M\n@THIS\nM=D\n");
    fprintf(fout, "@R6\nAM=M-1\nD=M\n@ARG\nM=D\n");
    fprintf(fout, "@R6\nAM=M-1\nD=M\n@LCL\nM=D\n");
    fprintf(fout, "@R7\nA=M\n0;JMP\n");
}

void writeFunction(char functionName[], int numLocals) {
    strcpy(currentFunction, functionName);
    fprintf(fout, "(%s)\n", functionName);

    for (int i = 0; i < numLocals; i++)
        pushConstant(0);
}

void translate() {
    if (command.type == C_PUSH || command.type == C_POP)
        writePushPop(command.type, command.segment, command.index);
    else if (command.type == C_ARITHMETIC)
        writeArithmetic(command.args[0]);
    else if (command.type == C_LABEL)
        writeLabel(command.label);
    else if (command.type == C_GOTO)
        writeGoto(command.label);
    else if (command.type == C_IF)
        writeIf(command.label);
    else if (command.type == C_CALL)
        writeCall(command.label, command.vars);
    else if (command.type == C_RETURN)
        writeReturn();
    else if (command.type == C_FUNCTION)
        writeFunction(command.label, command.vars);
}

void close() {
    fclose(fout);
}
