CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native
SRC_DIR = src
INCLUDE_DIR = -I $(SRC_DIR)/include -I lib/chess-library/include
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)
TARGET = silkfish

all: $(TARGET)
	@echo "Build complete. Cleaning up object files..."
	rm -f $(OBJ)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE_DIR)

clear:
	rm -f $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
