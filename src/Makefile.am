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
		    -I/usr/local/opt/gettext/include \
                    -DPATCHES_DIRECTORY=\"$(patchesdir)\"

# For local cJSON
if MOPO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/cJSON 
  cursynth_LDADD += ../cJSON/libcJSON.a
endif MOPO_LOCAL

# For local mopo
if MOPO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/mopo/src
  cursynth_LDADD += ../mopo/src/libmopo.a
endif MOPO_LOCAL

# For local rtaudio
if RTAUDIO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/rtaudio
  cursynth_LDADD += ../rtaudio/librtaudio.a
endif RTAUDIO_LOCAL

# For local rtmidi
if RTAUDIO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/rtmidi
  cursynth_LDADD += ../rtmidi/librtmidi.a 
endif RTAUDIO_LOCAL
