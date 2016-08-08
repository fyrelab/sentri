#!/bin/bash

BRANCH=`git rev-parse --abbrev-ref HEAD`

#webtool
if [ ! -d "sentri-webtool" ]; then
  git clone -b $BRANCH https://github.com/fyrelab/sentri-webtool sentri-webtool
else
  echo "Updating sentri webtool..."
  (cd sentri-webtool && git pull)
fi

if [ ! -d "dep" ]; then
  echo "Creating dependencies..."
  mkdir dep
fi

cd dep

if [ ! -d "boost_1_60_0" ]; then
  echo "Downloading boost..."
  curl http://netassist.dl.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz --output boost_1_60_0.tar.gz
  echo "Unpacking boost..."
  tar -xf boost_1_60_0.tar.gz
  rm -r boost_1_60_0.tar.gz
fi

if [ ! -d "libquickmail-0.1.22" ]; then
  echo "Downloading libquickmail..."
  curl -L https://sourceforge.net/projects/libquickmail/files/libquickmail-0.1.22.tar.xz --output libquickmail-0.1.22.tar.xz
  echo "Unpacking libquickmail..."
  tar xf libquickmail-0.1.22.tar.xz
  rm -f libquickmail-0.1.22.tar.xz
  cd libquickmail-0.1.22
  echo "Compiling library..."
  gcc -c -o quickmail.o quickmail.c
  ar rcs libquickmail.a quickmail.o
  cd ..
fi

#cpplint
if [ ! -d "styleguide" ]; then
  # git clone https://github.com/google/styleguide.git/
	git clone https://github.com/darkclouder/styleguide.git
else
  echo "Updating styleguide..."
  (cd styleguide && git pull)
fi

if [ ! -d "v4l2loopback" ]; then
  git clone https://github.com/umlaeute/v4l2loopback.git v4l2loopback
else
 (cd v4l2loopback)
fi

if [ ! -d "embeddedRest" ]; then
  git clone https://github.com/fnc12/embeddedRest.git embeddedRest
  (cd embeddedRest && git checkout 5529c98e5ba1dbad926a119d13e1b799ef2d3870)
else
 (cd embeddedRest)
fi


if [ ! -d "rapidjson" ]; then
  git clone https://github.com/miloyip/rapidjson.git rapidjson
  (cd rapidjson && git checkout 5e8a382c1978d3546349386063723cb713dbfcf9)

  rmdir embeddedRest/rapidjson
  ln -s ../rapidjson/include/rapidjson/ embeddedRest
else
  (cd rapidjson && git pull)
fi
