main :
	nasm -f elf64 checksum.asm -o checksum.o
	gcc checksum.o syn.c -o syn
	@echo "make is ok"
	@echo "please run the command : "
	@echo -e "\033[32m	./syn --help\033[0m"
.PHONY : clean
clean :
	@rm -f checksum.o syn
	@echo "make clean is ok"
