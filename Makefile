# The C++ compiler to use
CXX = g++

# Compiler flags:
# -std=c++17: Use the C++17 standard
# -Wall -Wextra: Enable all common and extra warnings
# -O2: Optimize the code for speed
# -g: Include debugging information (optional, but good practice)
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# The name of the final executable
TARGET = netflow-comp

# The single source file for the project
SOURCES = netflow-comp.cpp

# --- Rules ---

# The default goal, executed when you just run "make"
all: $(TARGET)

# Rule to link the object file into the final executable
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Rule to clean up the directory
# Removes the executable file
clean:
	rm -f $(TARGET)
	rm -rf netflow-comp.dSYM

# Phony targets are not real files
.PHONY: all clean