#!/bin/sh
set -x
d=${PWD}
triplet=""
extern_dir=""
is_mac=n
is_linux=n
is_win=n

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

# Detect system, set triplet. For now the triplet is hardcoded, w/o checks.
if [ "$(uname)" = "Darwin" ]; then
    is_mac=y
    triplet="mac-clang-x86_64"
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    is_linux=y
    triplet="linux-gcc-x86_64"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ]; then
    is_win=y
    triplet="win-vs2012-x86_64"
fi

extern_path=${d}/../extern/${triplet}
install_path=${d}/../install/${triplet}

# Check if extern dir is created and if not, compile dependencies.
if [  -d ${extern_path} ] ; then

    if [ "${is_linux}" = "y" ] ; then
        ./dependencies/build_unix_dependencies.sh
    elif [ "${is_mac}" = "y" ] ; then
        ./dependencies/build_unix_dependencies.sh
    fi

#    if [ ! -d ${d}/install ] ; then
#        echo "Cant find the `install` dir, something went wrong with the build dependencies script."
#        exit
#    fi
#
#    mkdir -p ${extern_path}
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
    ../ 

cmake --build . --target install --config Release

cd ./../../install/${triplet}/bin/
./kankerfont
