# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -std=c++17 -Idep/chowdsp_utils/modules/dsp -Idep/chowdsp_utils/modules/common -Idep/chowdsp_wdf/include
CFLAGS +=
CXXFLAGS +=

ifdef ARCH_MAC
# Obvioulsy get rid of this one day
	FLAGS += -Wno-c++11-extensions
endif


chowdsp := dep/libchowdsp/libchowdsp.a
OBJECTS += $(chowdsp)
DEPS += $(chowdsp)
$(chowdsp):
	git clone https://github.com/Chowdhury-DSP/chowdsp_utils dep/chowdsp_utils
	mkdir dep/libchowdsp
	touch dep/libchowdsp/CMakeLists.txt
	echo "add_subdirectory(../chowdsp_utils .)" >> dep/libchowdsp/CMakeLists.txt
	echo "setup_chowdsp_lib(chowdsp MODULES chowdsp_filters)" >> dep/libchowdsp/CMakeLists.txt
	echo "target_compile_features(chowdsp PRIVATE cxx_std_17)" >> dep/libchowdsp/CMakeLists.txt
	cd dep/libchowdsp && $(CMAKE) . -DCMAKE_BUILD_TYPE=Release && make -j16
	git clone https://github.com/Chowdhury-DSP/chowdsp_wdf dep/chowdsp_wdf

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

CXXFLAGS := $(filter-out -std=c++11,$(CXXFLAGS))