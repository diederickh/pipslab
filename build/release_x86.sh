#!/bin/sh
set -x
d=${PWD}
triplet=""
extern_dir=""
is_mac=n
is_linux=n
is_win=n
of=of_v0.8.4_vs_release
ofappdir=${d}/../${of}/apps/kankerfonds/abb

architecture="i386"
#export architecture="x86_64"

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


if [ "${is_win}" = "y" ] ; then 
    if [ ! -d ${d}/../${of} ] ; then
        cd ${d}/../
        #curl -o of.zip http://www.openframeworks.cc/versions/v0.8.4/of_v0.8.4_osx_release.zip
        curl -o of.zip http://www.openframeworks.cc/versions/v0.8.4/of_v0.8.4_vs_release.zip
        cd ${d}/../
        unzip ./of.zip
    fi

    if [ ! -d ${ofappdir} ] ; then
        mkdir -p ${ofappdir}
    fi

    if [ ! -d ${d}/../${of}/apps/kankerfonds ] ; then 
        cd ${d}/../src/
        echo "cmd /c mklink /J of .\..\\${of}\apps\kankerfonds\abb" > tmp.bat
        ./tmp.bat
    fi
fi


#if of_v0.8.4_osx_release
#cmd  /c 'mklink link target'

#cd ${d}/../
#unzip ${d}/../of.zip
#ls -alh  ${d}/../of.zip
#exit

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
