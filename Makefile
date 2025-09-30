# The C++ compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# The name of the final executable
TARGET = netflow-comp

# The source file
SOURCES = netflow-comp.cpp

# --- PcapPlusPlus Configuration ---
# Use the config script to get the necessary compiler and linker flags
PCPP_INCLUDES = $(shell pcapplusplus-config --includes)
PCPP_LIBS = $(shell pcapplusplus-config --libs)

# --- Rules ---

# Default goal
all: $(TARGET)

# Rule to link the object file into the final executable
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(PCPP_INCLUDES) $(PCPP_LIBS)

# Rule to clean up the directory
clean:
	rm -f $(TARGET)
	rm -rf netflow-comp.dSYM
	rm -rf pcap-comp.dSYM
	rm -f pcap-comp

# Phony targets are not real files
.PHONY: all clean