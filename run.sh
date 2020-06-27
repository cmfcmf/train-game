#!/bin/bash

VULKAN_SDK_PATH=/home/cmfcmf/dev/train-game/vendor/1.2.141.2/x86_64

LD_LIBRARY_PATH=$VULKAN_SDK_PATH/lib \
VK_LAYER_PATH=$VULKAN_SDK_PATH/etc/vulkan/explicit_layer.d \
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT \
build/TrainGame