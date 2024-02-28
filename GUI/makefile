# Makefile for GUI_Janneke
#libraries
BOOST   = /usr/include/boost
EIGEN   = /usr/include/eigen
BSO     = /home/kknopp/Documents/MSc_2023_Heuvelman
AEI	= /home/kknopp/Documents/MSc_2023_Heuvelman/BSO_grammars/AEI_Grammar
READS = /home/kknopp/Documents/MSc_2023_Heuvelman/BSO_grammars

ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO) -I$(AEI) -I$(READS)

# Compiler settings
CPP   = g++ -std=c++20 -fsanitize=address -g
FLAGS  = -O3 -lglut -lGL -lGLU

MAINFILE = GUI_Janneke.cpp

.PHONY: all cls clean

#make arguments
cls:
	# this argument clears the screen, so that it is easier to find the beginning of the compiler output
	@clear
	@clear
all:
	$(CPP) -o GUI_Janneke $(ALL_LIB) $(MAINFILE) $(FLAGS)
clean:
	@rm -f get_results
