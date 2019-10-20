# @file   configure.ac


AC_INIT([configure.ac])

m4_include([OpenSBE.ac])

OSBE_ENABLE_FEATURE([CmdDep])

OSBE_ENABLE_FEATURE([CXX/CXX])
OSBE_ENABLE_FEATURE([CXX/Ar])
OSBE_ENABLE_FEATURE([CXX/So])
OSBE_ENABLE_FEATURE([CXX/Inl])
OSBE_ENABLE_FEATURE([CXX/Ex])
OSBE_ENABLE_FEATURE([CXX/Def])
OSBE_ENABLE_FEATURE([CXX/AutoDep])

OSBE_ENABLE_FEATURE([CXX/XML/XSD])
OSBE_ENABLE_FEATURE([CXX/Corba])

OSBE_ENABLE_FEATURE([Documentation/Doxygen])


use_moran="no_tests"
osbe_ask_for_ne_string "Please enter \"full\", \"no_tests\" or \"none\"" \
                       "moran lib usage" \
                       use_moran \
                       "moran_status"
if [[ "X$use_moran" == "Xfull" ]]; then
  CPPFLAGS="$CPPFLAGS -DMORAN_TEST"
fi
if [[ "X$use_moran" != "Xnone" ]]; then
  CPPFLAGS="$CPPFLAGS -DMORAN_LIBRARY"
fi
AC_SUBST(use_moran)

use_klt="full"
osbe_ask_for_ne_string "Please enter \"full\" or \"none\"" \
                       "klt lib usage" \
                       use_klt \
                       "klt_status"
if [[ "X$use_klt" != "Xnone" ]]; then
  CPPFLAGS="$CPPFLAGS -DKLT_LIBRARY"
fi

use_mecab="full"
osbe_ask_for_ne_string "Please enter \"full\" or \"none\"" \
                       "mecab lib usage" \
                       use_mecab \
                       "mecab_status"
if [[ "X$use_mecab" != "Xnone" ]]; then
  CPPFLAGS="$CPPFLAGS -DMECAB_LIBRARY"
fi

use_java="full"
osbe_ask_for_ne_string "Please enter \"full\" or \"none\"" \
                       "java dependant parts" \
                       use_java \
                       "java_status"
AC_SUBST(use_java)

m4_include([dir.ac])

OSBE_OUTPUT
