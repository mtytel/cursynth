#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([cursynth], [1.6], [littleioaudio@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/cursynth.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX([c++ g++ cxx])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O
AM_PROG_AR
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.3])

# Checks for libraries.
AC_CHECK_LIB([intl], [gettext])
AC_CHECK_LIB([ncurses], [curs_set])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h libintl.h limits.h locale.h math.h ncurses.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h])

# Get flags for rtaudio and rtmidi.
AC_CANONICAL_HOST
AC_MSG_CHECKING(for audio API)
case $host in
  *-*-netbsd*)
    AC_MSG_RESULT(using OSS)
    api="$api -D__LINUX_OSS__"
    LIBS="$LIBS -lossaudio"
    AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(cursynth requires the pthread library!))
  ;;

  *-*-linux*)
  AC_ARG_WITH(jack, [  --with-jack = choose JACK server support (Mac OSX and GNU/Linux only)], [
    api="$api -D__UNIX_JACK__"
    AC_MSG_RESULT(using JACK)
    AC_CHECK_LIB(jack, jack_client_open, , AC_MSG_ERROR(JACK support requires the jack library!))
    AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(Jack support also requires the asound library!))], )

  # Look for ALSA flag
  AC_ARG_WITH(alsa, [  --with-alsa = choose native ALSA API support (GNU/Linux only)], [
    api="$api -D__LINUX_ALSA__"
    req="$req alsa"
    AC_MSG_RESULT(using ALSA)
    AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(ALSA support requires the asound library!))], )

  # Look for PULSE flag
  AC_ARG_WITH(pulse, [  --with-pulse = choose PulseAudio API support (GNU/Linux only)], [
    api="$api -D__LINUX_PULSE__"
    req="$req libpulse-simple"
    AC_MSG_RESULT(using PulseAudio)
    PKG_CHECK_MODULES([PULSE], [libpulse-simple], , AC_MSG_ERROR(PulseAudio support requires the pulse-simple library!))
        LIBS="$LIBS `pkg-config --libs libpulse-simple`" ], )

  # Look for OSS flag
  AC_ARG_WITH(oss, [  --with-oss = choose OSS API support (GNU/Linux only)], [
    api="$api -D__LINUX_OSS__"
    AC_MSG_RESULT(using OSS)], )

  # If no audio api flags specified, use ALSA
  if [test "$api" == "";] then
    AC_MSG_RESULT(using ALSA)
    AC_SUBST( api, [-D__LINUX_ALSA__] )
    req="$req alsa"
    AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(ALSA support requires the asound library!))
  fi

  AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(cursynth requires the pthread library!))
  ;;

  *-apple*)
  AC_ARG_WITH(jack, [  --with-jack = choose JACK server support (unix only)], [
    api="$api -D__UNIX_JACK__"
    AC_MSG_RESULT(using JACK)
    AC_CHECK_LIB(jack, jack_client_open, , AC_MSG_ERROR(JACK support requires the jack library!))], )

  # Look for Core flag
  AC_ARG_WITH(core, [  --with-core = choose CoreAudio API support (MAC OSX only)], [
    api="$api -D__MACOSX_CORE__"
    AC_MSG_RESULT(using CoreAudio)
    AC_CHECK_HEADER(CoreAudio/CoreAudio.h, [], [AC_MSG_ERROR(CoreAudio header files not found!)] )
    LIBS="$LIBS -framework CoreAudio -framework CoreMIDI -framework CoreFoundation" ], )

  # If no audio api flags specified, use CoreAudio
  if [test "$api" == ""; ] then
    AC_SUBST( api, [-D__MACOSX_CORE__] )
    AC_MSG_RESULT(using CoreAudio)
    AC_CHECK_HEADER(CoreAudio/CoreAudio.h,
      [],
      [AC_MSG_ERROR(CoreAudio header files not found!)] )
    LIBS="$LIBS -framework CoreAudio -framework CoreMIDI -framework CoreFoundation"
  fi

  AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(cursynth requires the pthread library!))
  ;;

  *-mingw32*)
  AC_ARG_WITH(asio, [  --with-asio = choose ASIO API support (Windows only)], [
    api="$api -D__WINDOWS_ASIO__"
    AC_MSG_RESULT(using ASIO)
    AC_SUBST( objects, ["asio.o asiodrivers.o asiolist.o iasiothiscallresolver.o"] ) ], )

  # Look for DirectSound flag
  AC_ARG_WITH(ds, [  --with-ds = choose DirectSound API support (Windows only)], [
    api="$api -D__WINDOWS_DS__"
    AC_MSG_RESULT(using DirectSound)
    LIBS="-ldsound -lwinmm $LIBS" ], )

  # If no audio api flags specified, use DirectSound
  if [test "$api" == "";] then
    AC_SUBST( api, [-D__WINDOWS_DS__] )
    AC_MSG_RESULT(using DirectSound)
    LIBS="-ldsound -lwinmm $LIBS"
  fi

  LIBS="-lole32 $LIBS"
  ;;

  *)
  # Default case for unknown realtime systems.
  AC_MSG_ERROR(Unknown system type for realtime support!)
  ;;
esac

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 floor gettimeofday memset modf pow rmdir strcasecmp strchr strdup strerror])

AC_CONFIG_SUBDIRS([mopo
                   rtaudio
                   rtmidi])

AC_CONFIG_FILES([Makefile po/Makefile.in
                 src/Makefile
                 cJSON/Makefile
                 doc/Makefile])
AC_OUTPUT
