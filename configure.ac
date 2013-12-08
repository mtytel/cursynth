#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([termite], [0.2], [littleioaudio@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/termite.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX([c++ g++])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([ncurses], [move])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h limits.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
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

AC_CONFIG_SUBDIRS(rtaudio
                  rtmidi)

AC_CONFIG_FILES([Makefile
                 mopo/Makefile
                 mopo/src/Makefile
                 src/Makefile
                 cJSON/Makefile])
AC_OUTPUT
