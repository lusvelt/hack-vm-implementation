VMtranslate: VMtranslate.o helper.o parser.o codeWriter.o includes.h
	gcc -o VMtranslate VMtranslate.o helper.o parser.o codeWriter.o

VMtranslate.o: VMtranslate.c helper.h parser.h codeWriter.h includes.h
	gcc -c VMtranslate.c

helper.o: helper.c helper.h parser.h codeWriter.h includes.h
	gcc -c helper.c

parser.o: parser.c helper.h parser.h codeWriter.h includes.h
	gcc -c parser.c

codeWriter.o: codeWriter.c helper.h parser.h codeWriter.h includes.h
	gcc -c codeWriter.c

debug:
	gcc -g -O0 VMtranslate.c helper.c parser.c codeWriter.c

clean:
	rm *.o VMtranslate *.out || del /Q /S *.o VMtranslate.exe *.out