BINARY = main

J_CPU  = stm32f103c8

LDSCRIPT = ../my_board.ld

include ../../Makefile.include

trace:
	$(TRACE)

program: $(BINARY).stlink-flash
	$(TRACE)