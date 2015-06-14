SUBDIRS = cJSON rtaudio rtmidi mopo src po doc
EXTRA_DIST = config.rpath patches

patchesdir = $(pkgdatadir)/patches
patches_DATA = $(top_srcdir)/patches/*.mite

ACLOCAL_AMFLAGS = -I m4
LDFLAGS += -L/usr/local/opt/gettext/lib
CPPFLAGS += -I/usr/local/opt/gettext/include
