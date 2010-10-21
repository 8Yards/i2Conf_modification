AC_INIT(src/main.cxx)
AM_INIT_AUTOMAKE(i2conf,0.8)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX
AM_SANITY_CHECK

AC_LANG_CPLUSPLUS


AC_CHECK_LIB([mutil],[main],,AC_MSG_ERROR([You need to have Minisip Util Library]),)
AC_CHECK_LIB([msip],[main],,AC_MSG_ERROR([You need to have Minisip Session Initiation Protocol (SIP) Library]),)
AC_CHECK_LIB([minisip],[main],,AC_MSG_ERROR([You need to have Minisip VoIP Application Support Library]),)

AC_CHECK_LIB([log4cxx],[main],,AC_MSG_ERROR([You need to have Boost C++ Library]),)

AC_CHECK_LIB([boost_thread], main, [], [
  AC_CHECK_LIB(boost_thread-mt, main, [], [
   AC_CHECK_LIB(boost_thread-gcc, main, [], [
    AC_CHECK_LIB(boost_thread-gcc-mt, main, [], [
      AC_CHECK_LIB(boost_thread-gcc-mt-s, main, [], [
        AC_MSG_ERROR([You need to have Boost C++ Library])
     ])
   ])
  ])
 ])
])

AC_CHECK_LIB([strmanager],[main],,AC_MSG_ERROR([You need to have strManager Library]),)

AC_OUTPUT(Makefile)
