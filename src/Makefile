CFLAGS = -D _DEBUG -ggdb3 -std=c++20 -O3 -mavx2 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr -pie -Wlarger-than=8192 -Wstack-usage=8192

SFML  = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
LOG_O = ../lib/logs/log.o
ALG_O = ../lib/algorithm/algorithm.o
#---------------------------------------------------------------------------
ALPHA 	  	= alpha_blending
ALPHA_H   	= $(ALPHA).h
ALPHA_CPP 	= $(ALPHA).cpp

PICTURE     = picture
PICTURE_H   = $(PICTURE).h
PICTURE_CPP = $(PICTURE).cpp

TIME        = timer
TIME_H      = $(TIME).h
TIME_CPP    = $(TIME).cpp

SET_H		= settings.h
#---------------------------------------------------------------------------
SOURCE = $(ALPHA_CPP) $(PICTURE_CPP)
HEADER = $(ALPHA_H)   $(PICTURE_H)   $(SET_H)
LIB	   = $(LOG_O)	  $(ALG_O)

all: blending timer

blending: main.cpp $(SOURCE) $(LIB) $(HEADER)
	g++   main.cpp $(SOURCE) $(LIB) $(CFLAGS) $(SFML) -o $@

timer:  $(TIME_CPP) $(TIME_H) $(ALPHA_CPP) $(ALPHA_H) $(LIB) $(SET_H)
	g++ $(TIME_CPP) $(CFLAGS) $(ALPHA_CPP)            $(LIB) -o $@

#---------------------------------------------------------------------------

clean:
	rm -f blending

.PHONY: clean