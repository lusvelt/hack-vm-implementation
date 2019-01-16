VMtranslate: VMtranslate.o helper.o parser.o includes.h
	gcc -o VMtranslate VMtranslate.o helper.o parser.o

VMtranslate.o: VMtranslate.c helper.h parser.h includes.h
	gcc -c VMtranslate.c

helper.o: helper.c helper.h parser.h includes.h
	gcc -c helper.c

parser.o: parser.c helper.h parser.h includes.h
	gcc -c parser.c

debug:
	gcc -g VMtranslate.c helper.c parser.c

clean:
	rm *.o VMtranslate || del /Q /S *.o VMtranslate.exe