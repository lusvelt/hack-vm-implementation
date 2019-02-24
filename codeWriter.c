#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];
char currentFunction[SYMBOL_MAX_LENGTH];
int comparisonsCount = 0;
int callsCount = 0;
int pushPop = 0;
int currentStatic = 0;
int maxIndex = 0;

void setFileName(const char* fileName) {

}

void setMaxStatic(int index) {
    if (index > maxIndex)
        maxIndex = index;
}

void prepareUnaryOperation() {
    fprintf(fout, "@SP\nA=M\nA=A-1\n");
}

void prepareBinaryOperation() {
    fprintf(fout, "@SP\nAM=M-1\nD=M\nA=A-1\n");
}

void operate(const char instructions[], int isBinary) {
    if (isBinary)
        prepareBinaryOperation();
    else
        prepareUnaryOperation();
    
    fprintf(fout, "%s", instructions);
}

void compare(const char comparison[]) {
    fprintf(fout, "@$$CMP:%d\nD=A\n@R13\nM=D\n@$$%s\n0;JMP\n($$CMP:%d)\n", comparisonsCount, comparison, comparisonsCount);

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
        compare("EQ");
    else if (strcmp(command, "gt") == 0)
        compare("GT");
    else if (strcmp(command, "lt") == 0)
        compare("LT");
    else if (strcmp(command, "and") == 0)
        operate("M=D&M\n", 1);
    else if (strcmp(command, "or") == 0)
        operate("M=D|M\n", 1);
    else if (strcmp(command, "not") == 0)
        operate("M=!M\n", 0);        
}

void writePushPop(enum commandType type, enum memorySegment segment, int index) {
    if (type == C_PUSH) {
        if (segment == M_CONSTANT)
            fprintf(fout, "@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", index);
        else if (segment == M_STATIC) {
            fprintf(fout, "@static:%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", currentStatic + index);
            setMaxStatic(index);
        } else if (segment == M_ARGUMENT)
            fprintf(fout, "@$$PUSH_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$PUSH_ARGUMENT\n0;JMP\n($$PUSH_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_LOCAL)
            fprintf(fout, "@$$PUSH_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$PUSH_LOCAL\n0;JMP\n($$PUSH_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_POINTER)
            fprintf(fout, "@R%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", 3 + index);
        else if (segment == M_THIS)
            fprintf(fout, "@$$PUSH_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$PUSH_THIS\n0;JMP\n($$PUSH_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_THAT)
            fprintf(fout, "@$$PUSH_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$PUSH_THAT\n0;JMP\n($$PUSH_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_TEMP)
            fprintf(fout, "@R%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", 5 + index);
    } else if (type == C_POP) {
        if (segment == M_CONSTANT)
            throwError("Cannot pop from segment 'constant'");
        else if (segment == M_STATIC) {
            fprintf(fout, "@SP\nAM=M-1\nD=M\n@static:%d\nM=D\n", currentStatic + index);
            setMaxStatic(index);
        } else if (segment == M_ARGUMENT)
            fprintf(fout, "@$$POP_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$POP_ARGUMENT\n0;JMP\n($$POP_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_LOCAL)
            fprintf(fout, "@$$POP_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$POP_LOCAL\n0;JMP\n($$POP_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_POINTER)
            fprintf(fout, "@SP\nAM=M-1\nD=M\n@R%d\nM=D\n", 3 + index);
        else if (segment == M_THIS)
            fprintf(fout, "@$$POP_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$POP_THIS\n0;JMP\n($$POP_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_THAT)
            fprintf(fout, "@$$POP_AFTER:%d\nD=A\n@R13\nM=D\n@%d\nD=A\n@$$POP_THAT\n0;JMP\n($$POP_AFTER:%d)\n", pushPop, index, pushPop);
        else if (segment == M_TEMP)
            fprintf(fout, "@SP\nAM=M-1\nD=M\n@R%d\nM=D\n", 5 + index);
    }
    pushPop++;
}

void writeLabel(char label[]) {
    fprintf(fout, "(%s$%s)\n", currentFunction, label);
}

void writeGoto(char label[]) {
    fprintf(fout, "@%s$%s\n0;JMP\n", currentFunction, label);
}

void writeIf(char label[]) {
    fprintf(fout, "@SP\nAM=M-1\nD=M\n@%s$%s\nD;JNE\n", currentFunction, label);
}

void writeCall(char functionName[], int numArgs) {
    fprintf(fout, "@$$endCall:%d\nD=A\n@R14\nM=D\n@return:%d\nD=A\n@$$CALL_%d\n0;JMP\n", callsCount, callsCount, numArgs);
    fprintf(fout, "($$endCall:%d)\n@%s\n0;JMP\n", callsCount, functionName);
    fprintf(fout, "(return:%d)\n", callsCount);

    callsCount++;
}

void writeReturn() {
    fprintf(fout, "@$$RET\n0;JMP\n");
}

void writeFunction(char functionName[], int numLocals) {
    strcpy(currentFunction, functionName);
    fprintf(fout, "(%s)\n", functionName);

    for (int i = 0; i < numLocals; i++)
        fprintf(fout, "@SP\nA=M\nM=0\n@SP\nM=M+1\n");
}

void translate() {
    // fprintf(fout, "// %s\n", currentCommand);
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

void writeInit() {
    fprintf(fout, "@256\nD=A\n@SP\nM=D\n@$$START$$\n0;JMP\n");

    // Return optimization
    fprintf(fout, "($$RET)\n");
    fprintf(fout, "@LCL\nD=M\n@5\nA=D-A\nD=M\n@R13\nM=D\n");
    fprintf(fout, "@SP\nA=M-1\nD=M\n@ARG\nA=M\nM=D\n");
    fprintf(fout, "D=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nAM=M-1\nD=M\n@THAT\nM=D\n");
    fprintf(fout, "@LCL\nAM=M-1\nD=M\n@THIS\nM=D\n");
    fprintf(fout, "@LCL\nAM=M-1\nD=M\n@ARG\nM=D\n");
    fprintf(fout, "@LCL\nAM=M-1\nD=M\n@LCL\nM=D\n");
    fprintf(fout, "@R13\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_0)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@5\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_1)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@6\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_2)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@7\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_3)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@8\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_4)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@9\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_5)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@10\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_6)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@11\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_7)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@12\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_8)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@13\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_9)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@14\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_10)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@15\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_11)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@16\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_12)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@17\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_13)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@18\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_14)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@19\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Call optimization
    fprintf(fout, "($$CALL_15)\n");
    fprintf(fout, "@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n");
    fprintf(fout, "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fprintf(fout, "@20\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n");
    fprintf(fout, "@SP\nD=M\n@LCL\nM=D\n@R14\nA=M\n0;JMP\n");

    // Eq optimization
    fprintf(fout, "($$EQ)\n");
    operate("D=M-D\n", 1);
    fprintf(fout, "@START_EQ\nD;JEQ\n");
    fprintf(fout, "@ELSE_EQ\n0;JMP\n");
    fprintf(fout, "(START_EQ)\n@SP\nA=M\nA=A-1\nM=-1\n");
    fprintf(fout, "@END_EQ\n0;JMP\n");
    fprintf(fout, "(ELSE_EQ)\n@SP\nA=M\nA=A-1\nM=0\n");
    fprintf(fout, "(END_EQ)\n");
    fprintf(fout, "@R13\nA=M\n0;JMP\n");

    // Gt optimization
    fprintf(fout, "($$GT)\n");
    operate("D=M-D\n", 1);
    fprintf(fout, "@START_GT\nD;JGT\n");
    fprintf(fout, "@ELSE_GT\n0;JMP\n");
    fprintf(fout, "(START_GT)\n@SP\nA=M\nA=A-1\nM=-1\n");
    fprintf(fout, "@END_GT\n0;JMP\n");
    fprintf(fout, "(ELSE_GT)\n@SP\nA=M\nA=A-1\nM=0\n");
    fprintf(fout, "(END_GT)\n");
    fprintf(fout, "@R13\nA=M\n0;JMP\n");

    // Lt optimization
    fprintf(fout, "($$LT)\n");
    operate("D=M-D\n", 1);
    fprintf(fout, "@START_LT\nD;JLT\n");
    fprintf(fout, "@ELSE_LT\n0;JMP\n");
    fprintf(fout, "(START_LT)\n@SP\nA=M\nA=A-1\nM=-1\n");
    fprintf(fout, "@END_LT\n0;JMP\n");
    fprintf(fout, "(ELSE_LT)\n@SP\nA=M\nA=A-1\nM=0\n");
    fprintf(fout, "(END_LT)\n");
    fprintf(fout, "@R13\nA=M\n0;JMP\n");

    // Push optimization
    fprintf(fout, "($$PUSH_ARGUMENT)\n");
    fprintf(fout, "@ARG\nA=D+M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$PUSH_LOCAL)\n");
    fprintf(fout, "@LCL\nA=D+M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$PUSH_THIS)\n");
    fprintf(fout, "@THIS\nA=D+M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$PUSH_THAT)\n");
    fprintf(fout, "@THAT\nA=D+M\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n@R13\nA=M\n0;JMP\n");

    // Pop optimization
    fprintf(fout, "($$POP_ARGUMENT)\n");
    fprintf(fout, "@ARG\nD=D+M\n@R14\nM=D\n@SP\nAM=M-1\nD=M\n@R14\nA=M\nM=D\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$POP_LOCAL)\n");
    fprintf(fout, "@LCL\nD=D+M\n@R14\nM=D\n@SP\nAM=M-1\nD=M\n@R14\nA=M\nM=D\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$POP_THIS)\n");
    fprintf(fout, "@THIS\nD=D+M\n@R14\nM=D\n@SP\nAM=M-1\nD=M\n@R14\nA=M\nM=D\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$POP_THAT)\n");
    fprintf(fout, "@THAT\nD=D+M\n@R14\nM=D\n@SP\nAM=M-1\nD=M\n@R14\nA=M\nM=D\n@R13\nA=M\n0;JMP\n");

    fprintf(fout, "($$START$$)\n");

    writeCall("Sys.init", 0);
}

void initializeStatics() {
    currentStatic += maxIndex + 1;
    maxIndex = 0;
}

void initializeCodeWriter(const char *outputFileName) {
    fout = fopen(outputFileName, "w");
    writeInit();
}

void close() {
    fclose(fout);
}
