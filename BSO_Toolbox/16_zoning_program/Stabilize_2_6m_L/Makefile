###################
# input variables #
###################
# -> program name
NAME 	 = zoning_stabilize_grammar
# -> source files to compile
ALLFILES = main.cpp
# -> location of the releases version of the toolbox
CUR_LIB  = /home/sjonnie/3_projects/16_zoning_program/dennis_toolbox
# -> location of the development branch of the toolbox
DEV_LIB  = /home/sjonnie/4_dev_toolbox
# -> location of the eigen libraries
EIGEN    = /usr/include/eigen
# -> location of the boost libraries
BOOST    = /usr/include/boost

####################################
# Compiler flags, don't touch this #
####################################
# -> which compiler will be used
CC 	     = g++ -std=c++11
# -> common flags (visualisation and multithreading)
ALLFLAGS = -lglut -lGL -lGLU -lpthread
# -> release flags
R_FLAGS  = -O3 -march=native
# -> debug flags
D_FLAGS  = -g -Og -Wall
# -> location of the grammars
GRAMMARS = /BSO\ grammars
# -> location of the toolbox
TOOLBOX  = /BSO\ Toolbox
# -> program name for release version
CUR_NAME = $NAME
# -> program name for development version
DEV_NAME = $(NAME)_dev
# -> program name for debug version
DBG_NAME = $(NAME)_dbg

####################################
# make flags --> calls to compiler #
####################################
.PHONY: cls all dbg dev clean
cls:
	@clear
	@clear
all:
	$(CC) $(ALLFILES) -o $(NAME) -I$(CUR_LIB)$(GRAMMARS) -I$(CUR_LIB)$(TOOLBOX) -I$(EIGEN) -I$(BOOST) $(ALLFLAGS) $(R_FLAGS)
dbg:
	$(CC) $(ALLFILES) -o $(DBG_NAME) -I$(DEV_LIB)$(GRAMMARS) -I$(DEV_LIB)$(TOOLBOX) -I$(EIGEN) -I$(BOOST) $(ALLFLAGS) $(D_FLAGS)
dev:
	$(CC) $(ALLFILES) -o $(DEV_NAME) -I$(DEV_LIB)$(GRAMMARS) -I$(DEV_LIB)$(TOOLBOX) -I$(EIGEN) -I$(BOOST) $(ALLFLAGS) $(R_FLAGS)
clean:
	@rm -f $(NAME)
	@rm -f $(DEV_NAME)
	@rm -f $(DBG_NAME)