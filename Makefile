all:
	make -C firmware
	#gcc -std=c99 pa-test.c firmware/usynth.c -o pa-test -lpulse-simple `pkg-config --cflags --libs gtk+-2.0 glib-2.0 cairo`
	gcc -std=c99 tests.c firmware/usynth.c -o tests -lpulse-simple 
	gcc -std=c99 client/main.c driver/usynth.c -o uplay -lrt
	
install:
	make -C firmware install

fuse:
	make -C firmware fuse

tests: 
	make -C . all
	./tests | sox -t raw -r 16384 -uLb 8 -c 1 - output.wav
	
clean:
	make -C firmware clean
