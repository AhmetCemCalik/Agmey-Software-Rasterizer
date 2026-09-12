CXX = g++
# Fixed -I/src to -Isrc so it targets the local project folder
CXXFLAGS = -std=c++20 -g -I/opt/homebrew/include -Isrc -O3
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf

# Updated to look inside src/ from the project root
SRC = $(wildcard src/*.cpp) \
      $(wildcard src/core/*.cpp) \
      $(wildcard src/player/*.cpp) \
      $(wildcard src/rendering/*.cpp) \
      $(wildcard src/world/*.cpp)

# Define the build directory and output target
BUILD_DIR = bin
TARGET = $(BUILD_DIR)/main-doom

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)