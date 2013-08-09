# Copyright 2013 Little IO
#
# laf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# laf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with laf.  If not, see <http://www.gnu.org/licenses/>.

TERMITE = termite

LAF_DIR = ../laf
OUTPUT_DIR = debug
SOURCE_DIR = src
RTAUDIO_DIR = rtaudio
RTMIDI_DIR = rtmidi

CXXFLAGS += -g -Wall -Wextra -O3
CXX_INCLUDES = -I $(LAF_DIR)/src/ -I $(RTAUDIO_DIR) \
               -I $(RTMIDI_DIR) -I $(SOURCE_DIR)
CXX_LIBS = $(LAF_DIR)/laf.a -lpthread -lncurses

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

all: directory $(TERMITE)

clean:
	rm -rf $(OUTPUT_DIR) $(TERMITE)

# Build Directories
directory:
	@mkdir -p $(OUTPUT_DIR)/$(SOURCE_DIR)

# Building
$(TERMITE): $(OBJS) $(OUTPUT_DIR)/rtaudio.o $(OUTPUT_DIR)/rtmidi.o
	@echo 'Creating $@'
	@$(CXX) $(CXXFLAGS) $^ -o $(TERMITE) $(CXX_LIBS) $(OS_LINK_FLAGS)
	@echo 'Complete'
	@echo ' '

$(OUTPUT_DIR)/$(SOURCE_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@

$(OUTPUT_DIR)/rtaudio.o: rtaudio/RtAudio.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@

$(OUTPUT_DIR)/rtmidi.o: rtmidi/RtMidi.cpp
	@echo 'Building $@'
	@$(CXX) $(CXXFLAGS) $(CXX_INCLUDES) $(OS_COMPILE_FLAGS) -c $< -o $@
