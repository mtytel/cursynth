bin_PROGRAMS = termite

termite_SOURCES = main.cpp \
									termite.cpp \
									termite_gui.cpp \
									termite_strings.cpp \
									termite_synth.cpp

termite_CPPFLAGS = -I$(top_srcdir)/cJSON \
									 -I$(top_srcdir)/rtaudio \
									 -I$(top_srcdir)/rtmidi \
									 -I$(top_srcdir)/mopo/src

termite_LDFLAGS = -framework CoreMIDI \
									-framework CoreAudio \
									-framework CoreFoundation

termite_LDADD = ../cJSON/libcJSON.a \
								../rtaudio/librtaudio.a \
								../rtmidi/librtmidi.a \
								../mopo/src/libmopo.a
