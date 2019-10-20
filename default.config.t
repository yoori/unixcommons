#if defined(MORAN_FULL)
moran_status="full"
#elif defined(MORAN_TESTS)
moran_status="no_tests"
#else
moran_status="none"
#endif

#ifdef KLT_FULL
klt_status="full"
#else
klt_status="none"
#endif

#ifdef MECAB_FULL
mecab_status="full"
#else
mecab_status="none"
#endif

#ifdef NOJAVA
java_status="none"
#else
java_status="full"
#endif

#if DEBUG
#define OPT_FLAGS -ggdb3 -O0 -fno-inline -DDEV_DEBUG -rdynamic
#else
#define OPT_FLAGS -g -O3 -Werror
#endif

cxx.id = GCC
cxx.cxx = g++
cxx.cpp_flags = -std=c++0x -D_REENTRANT -m64 -march=x86-64 -fPIC -DPIC
cxx.flags = -pthread -W -Wall OPT_FLAGS
cxx.obj.ext = .o

cxx.ld.flags = -pthread -W -Wall OPT_FLAGS -m64 -march=x86-64
cxx.ld.libs =
cxx.so.ld = g++ -shared
cxx.so.primary.target.templ = lib%.so
cxx.so.secondary.target.templ =
cxx.ex.ld = g++
cxx.ex.target.templ = %

cxx.corba.orb.id = TAO
cxx.corba.orb.dir = /none
cxx.corba.orb.idl = /usr/bin/tao_idl
cxx.corba.cppflags = -I.
cxx.corba.cxxflags =
cxx.corba.ldflags =
cxx.corba.skelsuffix = _s
cxx.corba.stubsuffix =
cxx.corba.idlppflags =
cxx.corba.extraidlflags =
//external.lib.tao_openssl.root = /usr

cxx.xml.xsd.id = XSD
cxx.xml.xsd.dir = /none
cxx.xml.xsd.translator = /usr/bin/xsd
cxx.xml.xsd.cppflags = -DXSD_CXX11
cxx.xml.xsd.cxxflags =
cxx.xml.xsd.ldflags =
cxx.xml.xsd.extra_translator_flags = --std c++11
//external.lib.xsd_xerces.root = /usr

documentation.doxygen.path = doxygen

//external.lib.ace.root = /usr
external.lib.apache.root = /usr
external.lib.apr.root = /usr
//external.lib.bzip2.root = /usr
//external.lib.crypto.root = /usr
//external.lib.dynloader.root = /usr
//external.lib.event.root = /usr
//external.lib.geoip.root = /usr
//external.lib.gzip.root = /usr
external.lib.java.root = /usr/lib/jvm/java
external.lib.klt.root = /opt/KLT
//external.lib.mecab.root = /usr
external.lib.moran.root = /opt/Moran
//external.lib.netsnmp.root = /usr
//external.lib.nlpir.root = /usr
//external.lib.openssl.root = /usr
//external.lib.pcre.root = /usr
//external.lib.xerces.root = /usr
