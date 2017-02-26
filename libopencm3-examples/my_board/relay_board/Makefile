BINARY = main

J_CPU  = stm32f103c8

LDSCRIPT = ../my_board.ld

include ../../Makefile.include

trace:
	$(J_TRACE) -device $(J_CPU) -if swd

program: $(BINARY).stlink-flash
	$(TRACE)