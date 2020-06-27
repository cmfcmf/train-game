VULKAN_SDK_PATH = /home/cmfcmf/dev/train-game/vendor/1.2.141.2/x86_64

CXX      := -c++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -std=c++17 -DGLM_FORCE_RADIANS
LDFLAGS  := -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/apps
TARGET   := TrainGame
INCLUDE  := -I$(VULKAN_SDK_PATH)/include
SRC      := $(wildcard src/*.cpp)

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPS     := $(OBJECTS:.o=.d)

-include $(DEPS)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -c $< -o $@ $(LDFLAGS)

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

.PHONY: all build clean debug release run

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g -O0
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

run: all
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT $(APP_DIR)/$(TARGET)
