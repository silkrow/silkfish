CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -pthread
SRC_DIR = src
INCLUDE_DIR = -I $(SRC_DIR)/include -I lib/chess-library/include
OBJ = main.o evaluation.o search.o input_parser.o uci.o

all: chess-engine

chess-engine: $(OBJ)
	$(CXX) $(CXXFLAGS) -o silkfish $(OBJ)

main.o: $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp $(INCLUDE_DIR)

evaluation.o: $(SRC_DIR)/evaluation.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/evaluation.cpp $(INCLUDE_DIR)

search.o: $(SRC_DIR)/search.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/search.cpp $(INCLUDE_DIR)

input_parser.o: $(SRC_DIR)/input_parser.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/input_parser.cpp $(INCLUDE_DIR)

uci.o: $(SRC_DIR)/uci.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/uci.cpp $(INCLUDE_DIR)

clean:
	rm -f *.o silkfish
