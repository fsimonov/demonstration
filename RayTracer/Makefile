CXX = g++

CXXFLAGS = -g -Wall -O2 -std=c++11

MAIN_TARGET = ./bin/RayTracer

SRC_DIR = src
INCLUDE_DIR = ./src/include
OBJ_DIR = ./bin/obj
DEPS_DIR = ./bin/deps
EXTERNALS_DIR = ./bin/externals
BRIDGE_DIR = ./bin/bridge

src_to_obj = $(addprefix $(OBJ_DIR)/, $(notdir $(1:.cpp=.o)))

SRC_MODULES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_MODULES = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC_MODULES:.cpp=.o)))
DEPS_MODULES = $(addprefix $(DEPS_DIR)/, $(notdir $(SRC_MODULES:.cpp=.d)))

BRIDGE_MAKE = $(BRIDGE_DIR)/Makefile
BRIDGE_INCLUDE_DIR = $(BRIDGE_DIR)/include
BRIDGE_LIBRARY_DIR = $(BRIDGE_DIR)/lib

BRIDGE_TARGETS = FreeImage

LDFLAGS = -lfreeimage
LDFLAGS += -L $(BRIDGE_LIBRARY_DIR)

CXXFLAGS += -I $(INCLUDE_DIR) -I $(BRIDGE_INCLUDE_DIR)
CXXFLAGS += -I $(EXTERNALS_DIR)/glm
CXXFLAGS += -I $(EXTERNALS_DIR)/FreeImagePlus

FIP_SRC = $(wildcard $(EXTERNALS_DIR)/FreeImagePlus/src/*.cpp)
FIP_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(FIP_SRC:.cpp=.o)))

all: $(MAIN_TARGET)

ifneq (clean, $(MAKECMDGOALS))
-include bridge.touch
endif

$(MAIN_TARGET): $(OBJ_MODULES) $(FIP_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(MAIN_TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(EXTERNALS_DIR)/FreeImagePlus/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEPS_DIR)/%.d: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -E -MM -MT $(call src_to_obj, $<) -MT $@ -MF $@ $<

bridge.touch:
	mkdir -p $(OBJ_DIR) $(DEPS_DIR)
	mkdir -p $(BRIDGE_INCLUDE_DIR) $(BRIDGE_LIBRARY_DIR)
	make -C $(dir $(BRIDGE_MAKE)) -f $(notdir $(BRIDGE_MAKE)) $(BRIDGE_TARGETS)
	echo "-include $(DEPS_MODULES)" > $@

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(DEPS_DIR)
	make -C $(dir $(BRIDGE_MAKE)) -f $(notdir $(BRIDGE_MAKE)) clean
	rm -rf $(BRIDGE_INCLUDE_DIR)
	rm -rf $(BRIDGE_LIBRARY_DIR)
	rm -f bridge.touch
	rm -f $(MAIN_TARGET)
	rm -rf $(MAIN_TARGET).dSYM

.PHONY: all clean