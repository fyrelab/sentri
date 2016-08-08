# system name
SYSTEM_NAME=sentri

# Root dir
ROOT_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Compiler
CC=g++

# Flags for all builds
CFLAGS=-std=c++11

# Flags for release build
R_CFLAGS=$(CFLAGS) -Wall -O2

# Flags for debug build
D_CFLAGS=$(CFLAGS) -g -Wall -DDEV
T_CFLAGS=$(CFLAGS) -g -Wall -DDEV -DTESTING

UNAME=$(shell uname)

# Linking flags
D_LFLAGS=
R_LFLAGS=
LFLAGS=
LIBS=-lboost_system -lboost_filesystem -lboost_chrono -lboost_date_time


# Darwin = MacOS
ifeq ($(UNAME),Darwin)
   PYTHON=python
	 LFLAGS+=-L/usr/local/lib/
	 LIBS+=-lboost_thread-mt
else
   PYTHON=python2
   LIBS+=-lrt -lpthread -lboost_thread
endif

INCLUDES=-I$(ROOT_DIR)/dep/boost_1_60_0/ -I$(ROOT_DIR)/dep/libquickmail-0.1.22/ -I$(ROOT_DIR)/src/
