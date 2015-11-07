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
if CJSON_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/cJSON 
  if !cursynth_LDADD
    cursynth_LDADD = ../cJSON/libcJSON.a
  else
    cursynth_LDADD += ../cJSON/libcJSON.a
  endif
endif CJSON_LOCAL

# For local mopo
if MOPO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/mopo/src
  if !cursynth_LDADD
    cursynth_LDADD = ../mopo/src/libmopo.a
  else
    cursynth_LDADD += ../mopo/src/libmopo.a
  endif
endif MOPO_LOCAL

# For local rtaudio
if RTAUDIO_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/rtaudio
  if !cursynth_LDADD
    cursynth_LDADD = ../rtaudio/librtaudio.a
  else
    cursynth_LDADD += ../rtaudio/librtaudio.a
  endif 
endif RTAUDIO_LOCAL

# For local rtmidi
if RTMIDI_LOCAL
  cursynth_CPPFLAGS += -I$(top_srcdir)/rtmidi
  if !cursynth_LDADD
    cursynth_LDADD = ../rtmidi/librtmidi.a 
  else 
    cursynth_LDADD += ../rtmidi/librtmidi.a 
  endif
endif RTMIDI_LOCAL
