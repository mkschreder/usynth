all:
	make -C firmware

install:
	make -C firmware install

fuse:
	make -C firmware fuse
	
clean:
	make -C firmware clean
