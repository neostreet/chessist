chessist: chessist.o chessatk.o chessdbg.o chessmsc.o chessmvs.o chessrd.o bitfuns.o ioutil.o chessist.res
	g++ -mwindows -g -L"/cygdrive/c/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x86" -o chessist chessist.o chessatk.o chessmsc.o chessdbg.o chessmvs.o chessrd.o ioutil.o bitfuns.o chessist.res -lGdi32 -lcurses -lComDlg32 -lComCtl32

chessist.o: chessist.c
	g++ -g -c -I./common chessist.c

chessatk.o: ./common/chessatk.c
	g++ -g -c -I./common ./common/chessatk.c

chessdbg.o: ./common/chessdbg.c
	g++ -g -c -I./common ./common/chessdbg.c

chessmsc.o: ./common/chessmsc.c
	g++ -g -c -I./common ./common/chessmsc.c

chessmvs.o: ./common/chessmvs.c
	g++ -g -c -I./common ./common/chessmvs.c

chessrd.o: ./common/chessrd.c
	g++ -g -c -I./common ./common/chessrd.c

bitfuns.o: ./common/bitfuns.c
	g++ -g -c -I./common ./common/bitfuns.c

ioutil.o: ./common/ioutil.c
	g++ -g -c -I./common ./common/ioutil.c

chessist.res: ./common/chessist.rc
	windres ./common/chessist.rc -O coff -o chessist.res
