CXX=g++
CXX_FLAGS=-g -O2 -c -std=c++14
CXX_SRC=main.cpp
CXX_OBJ=$(CXX_SRC:.cpp=.o)

TARGET=ns

default: $(CXX_SRC)
	make all

all: $(CXX_OBJ)
	$(CXX) $(CXX_OBJ) -g -o $(TARGET)

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) $< -o $@
	
