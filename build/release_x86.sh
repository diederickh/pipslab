#!/bin/sh
set -x
d=${PWD}
triplet=""
extern_dir=""
is_mac=n
is_linux=n
is_win=n

#export architecture="i386"
export architecture="x86_64"

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

# Detect system, set triplet. For now the triplet is hardcoded, w/o checks.
if [ "$(uname)" = "Darwin" ]; then
    is_mac=y
    triplet="mac-clang-${architecture}"
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    is_linux=y
    triplet="linux-gcc-${architecture}"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ]; then
    is_win=y
    triplet="win-vs2012-${architecture}"
fi

extern_path=${d}/../extern/${triplet}
install_path=${d}/../install/${triplet}

# Checkout the dependencies module
if [ ! -d ${d}/dependencies ] ; then
    git clone git@github.com:roxlu/dependencies.git
fi

# Check if extern dir is created and if not, compile dependencies.
if [ ! -d ${extern_path} ] ; then

    if [ "${is_linux}" = "y" ] ; then
        ./dependencies/build_unix_dependencies.sh
    elif [ "${is_mac}" = "y" ] ; then
        ./dependencies/build_unix_dependencies.sh
    elif [ "${is_win}" = "y" ] ; then
        ./dependencies/build_win_dependencies.sh
    fi
fi

# Compile the library.
cd build.release

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    -DEXTERN_LIB_DIR=${extern_path}/lib \
    -DEXTERN_INC_DIR=${extern_path}/include \
    -DEXTERN_SRC_DIR=${extern_path}/src \
    -DTINYLIB_DIR=${d}/sources/tinylib \
    -DCMAKE_OSX_ARCHITECTURES=${architecture} \
    ../ 

cmake --build . --target install --config Release

cd ./../../install/${triplet}/bin/
./kankerfont
#./kankerabb
#./test_socket
#./test_socket_abb
