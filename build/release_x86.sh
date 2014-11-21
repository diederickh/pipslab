#!/bin/bash

d=${PWD}
my_build_dir=${d}

# Checkout the dependencies module
if [ ! -d ${d}/dependencies ] ; then
    git clone git@github.com:roxlu/dependencies.git
fi

# Set environment variables

vs="2012"
source ./dependencies/build.sh

set -x

# Download OF for win.
if [ "${is_win}" = "y" ] ; then
    of=of_v0.8.4_vs_release
    ofappdir=${d}/../${of}/apps/kankerfonds/

    if [ ! -d ${d}/../${of} ] ; then
        cd ${d}/../
        if [ ! -f of.zip ] ; then
            curl -o of.zip http://www.openframeworks.cc/versions/v0.8.4/of_v0.8.4_vs_release.zip
        fi

        cd ${d}/../
        if [ ! -d ${of} ] ; then
          unzip ./of.zip
        fi
    fi

    if [ ! -d ${ofappdir} ] ; then
        mkdir -p ${ofappdir}
    fi

    # Create the link
    if [ ! -d ${ofappdir}/abb ] ; then
        cd ${ofappdir}
        link abb ./../../../src/of/
    fi
fi

if [ ! -d build.release ] ; then
    mkdir build.release
fi

# Compile the library.
cd ${my_build_dir}
cd build.release

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    -DEXTERN_LIB_DIR=${extern_path}/lib \
    -DEXTERN_INC_DIR=${extern_path}/include \
    -DEXTERN_SRC_DIR=${extern_path}/src \
    -DTINYLIB_DIR=${d}/sources/tinylib \
    -DREMOXLY_DIR=${sd}/remoxly \
    ${cmake_osx_architextures} \
    -G "${cmake_generator}" \
    ../

cmake --build . --target install --config Release

cd ${install_path}/bin/
./kankerfont
#./kankerabb
#./test_socket
#./test_socket_abb
