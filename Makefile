VMtranslator: VMtranslator.o helper.o parser.o codeWriter.o includes.h
	gcc -o VMtranslator VMtranslator.o helper.o parser.o codeWriter.o

VMtranslator.o: VMtranslator.c helper.h parser.h codeWriter.h includes.h
	gcc -c VMtranslator.c

helper.o: helper.c helper.h parser.h codeWriter.h includes.h
	gcc -c helper.c

parser.o: parser.c helper.h parser.h codeWriter.h includes.h
	gcc -c parser.c

codeWriter.o: codeWriter.c helper.h parser.h codeWriter.h includes.h
	gcc -c codeWriter.c

debug:
	gcc -g -O0 VMtranslator.c helper.c parser.c codeWriter.c

clean:
	rm *.o VMtranslator *.out || del /Q /S *.o VMtranslator.exe *.out