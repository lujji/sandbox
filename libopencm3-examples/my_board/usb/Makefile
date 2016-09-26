##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY = usb

LDSCRIPT = ../my_board.ld

include ../../Makefile.include

J_LINK = /home/vlm/local/JLink/JLinkExe
OOCD   = /home/lujji/local/openocd/bin/openocd

jlink-flash: $(BINARY).bin
	@printf "Starting J-Link..\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	@printf "connect\nr\nloadbin \"$(BINARY).bin\", 0x8000000\nr\nq\n" > flash.jlink
	$(J_LINK) -device stm32f103c8 -if swd -speed 4000 ./flash.jlink

program: $(BINARY).stlink-flash

oocd: $(BINARY).elf
	$(OOCD) -f config.cfg -c "program $(BINARY).elf reset verify exit"
