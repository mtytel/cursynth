SUBDIRS = cJSON rtmidi po doc
EXTRA_DIST = config.rpath patches

# For local rtaudio
if RTAUDIO_LOCAL
  SUBDIRS += rtaudio
endif RTAUDIO_LOCAL

# For local mopo
if MOPO_LOCAL
  SUBDIRS += mopo
endif MOPO_LOCAL

SUBDIRS += src

patchesdir = $(pkgdatadir)/patches
patches_DATA = $(top_srcdir)/patches/*.mite

ACLOCAL_AMFLAGS = -I m4
LDFLAGS += -L/usr/local/opt/gettext/lib
CPPFLAGS += -I/usr/local/opt/gettext/include
