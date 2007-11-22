# (c) Dr. I. T. Hernadvolgyi, EU.EDGE LLC
#
# This Makefile is absolutely free in any sense of the word
# (including license free).
# Please adopt it as you see fit.
#
# Note that the source code files are free but NOT license free.
# 
# TARGETS:
#
#     ua (default): builds the ua program  
#     all         : builds the ua program and static and shared libraries
#     install     : installs the ua program (if built), the libraries 
#                   (if exist), the header file (if the libraries exist)
#                   and the manpage at the usual places under $(PREFIX)
#     uninstall   : removes the files under $(PREFIX)
#

VERSION=1.0
PREFIX=/usr/local
CXX=g++
CFLAGS= -Wall -O3 -pedantic
LDFLAGS= -lcrypto
#DEFINES = -D__NOHASH

ua: ua.cc filei.h filei.cc filei.h
	$(CXX) -o ua $(CFLAGS) $(DEFINES) ua.cc filei.cc -I. $(LDFLAGS)

all: ua lib

install: ua
	if [ -f ua ]; then mkdir -p $(PREFIX)/bin; cp ua $(PREFIX)/bin; fi
	mkdir -p $(PREFIX)/share/man/man1; cp ua.1 $(PREFIX)/share/man/man1
	if [ -f libfilei.a ]; then mkdir -p $(PREFIX)/lib; cp libfilei.a $(PREFIX)/lib; fi
	if [ -f libfilei.so ]; then mkdir -p $(PREFIX)/lib; cp libfilei.so $(PREFIX)/lib; fi
	if [ -f libfile.a -o -f libfilei.so ]; then mkdir -p $(PREFIX)/include; cp filei.h $(PREFIX)/include; fi

uninstall:
	rm -f $(PREFIX)/bin/ua
	rm -f $(PREFIX)/share/man/man1/ua.1
	rm -f $(PREFIX)/lib/libfilei.a $(PREFIX)/lib/libfilei.so
	rm -f $(PREFIX)/include/filei.h


lib: libfilei.a libfilei.so

libfilei.a: filei.o
	ar cvr $@ filei.o
	ranlib $@ 

libfilei.so: filei.o
	$(CXX) -shared -o $@ filei.o 

filei.o: filei.cc filei.h
	$(CXX) -c filei.cc $(CFLAGS) $(DEFINES) -I. -fPIC

clean:
	rm -f ./ua ./filei.o ./libfilei.a ./libfilei.so

dist:
	mkdir ua-$(VERSION)
	cp ua.cc filei.h filei.cc Makefile README ua.1 ua-$(VERSION)
	tar cfz ua-$(VERSION).tar.gz ua-$(VERSION)
	rm -rf ua-$(VERSION)
