bin_PROGRAMS = termite

termite_SOURCES = main.cpp \
									termite.cpp \
									termite_gui.cpp \
									termite_strings.cpp \
									termite_synth.cpp \
									termite.h \
									termite_common.h \
									termite_gui.h \
									termite_strings.h \
									termite_synth.h

termite_CPPFLAGS = -I$(top_srcdir)/cJSON \
									 -I$(top_srcdir)/rtaudio \
									 -I$(top_srcdir)/rtmidi \
									 -I$(top_srcdir)/mopo/src

termite_LDADD = ../cJSON/libcJSON.a \
								../rtaudio/librtaudio.a \
								../rtmidi/librtmidi.a \
								../mopo/src/libmopo.a
