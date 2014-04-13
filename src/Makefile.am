bin_PROGRAMS = cursynth
patchesdir = $(pkgdatadir)/patches

cursynth_SOURCES = main.cpp \
                   cursynth.cpp \
                   cursynth_engine.cpp \
                   cursynth_gui.cpp \
                   cursynth_strings.cpp \
                   cursynth.h \
                   cursynth_common.h \
                   cursynth_engine.h \
                   cursynth_gui.h \
                   cursynth_strings.h

cursynth_CPPFLAGS = -I. \
                    -I.. \
                    -I$(top_srcdir)/cJSON \
                    -I$(top_srcdir)/rtaudio \
                    -I$(top_srcdir)/rtmidi \
                    -I$(top_srcdir)/mopo/src \
                    -I$(top_srcdir)/mopo/src \
										-I/usr/local/opt/gettext/include \
                    -DPATCHES_DIRECTORY=\"$(patchesdir)\"

cursynth_LDADD = ../cJSON/libcJSON.a \
                 ../rtaudio/librtaudio.a \
                 ../rtmidi/librtmidi.a \
                 ../mopo/src/libmopo.a
