# Copyright 2013 Little IO
#
# terminte is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# terminte is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with terminte.  If not, see <http://www.gnu.org/licenses/>.

SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

TERMITE = termite

MOPO_DIR = mopo
OUTPUT_DIR = debug
SOURCE_DIR = src
RTAUDIO_DIR = rtaudio
RTMIDI_DIR = rtmidi
CJSON_DIR = cJSON

CXXFLAGS += -g -Wall -Wextra -O3
CXX_INCLUDES = -I $(MOPO_DIR)/src/ -I $(RTAUDIO_DIR) \
               -I $(RTMIDI_DIR) -I $(CJSON_DIR) -I $(SOURCE_DIR)
CXX_LIBS = $(MOPO_DIR)/mopo.a -lpthread -lncurses

UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
OS_COMPILE_FLAGS = -D__LINUX_ALSA__
OS_LINK_FLAGS = -lasound
else
OS_COMPILE_FLAGS = -D__MACOSX_CORE__
OS_LINK_FLAGS = -framework CoreMIDI -framework CoreAudio \
                -framework CoreFoundation
endif

OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(OUTPUT_DIR)/$(SOURCE_DIR)/%.o, \
        $(wildcard $(SOURCE_DIR)/*.cpp))

all: mopo_lib directory $(TERMITE)

clean:
	@$(MAKE) clean -C $(MOPO_DIR)
	@echo 'Cleaning $(TERMITE) build files'
	@rm -rf $(OUTPUT_DIR) $(TERMITE)

# Build Directories
directory:
	@mkdir -p $(OUTPUT_DIR)/$(SOURCE_DIR)

# Building
mopo_lib:
	@echo 'Building $(MOPO_DIR)'
	@$(MAKE) -C $(MOPO_DIR)

$(TERMITE): $(OBJS) $(OUTPUT_DIR)/cJSON.o \
            $(OUTPUT_DIR)/rtaudio.o $(OUTPUT_DIR)/rtmidi.o
	@echo 'Creating $@'
	@$(CXX) $(CXXFLAGS) $^ -o $(TERMITE) $(CXX_LIBS) $(OS_LINK_FLAGS)
	@echo 'Complete'
	@echo ' '

$(OUTPUT_DIR)/$(SOURCE_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@

$(OUTPUT_DIR)/rtaudio.o: $(RTAUDIO_DIR)/RtAudio.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@

$(OUTPUT_DIR)/rtmidi.o: $(RTMIDI_DIR)/RtMidi.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@

$(OUTPUT_DIR)/cJSON.o: $(CJSON_DIR)/cJSON.c
	@echo 'Building $@'
	@$(CC) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@
