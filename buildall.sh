#/bin/bash
function cmakebuild {
    local d=$1
    pushd .
    mkdir -p ../$d/build
    cd ../$d/build
    rm -rf *
    cmake ..
    make -j8 install
    rm -rf *
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j8 install
    popd
}

function botanbuild {
    pushd .
    cd ../cppssh
    ./buildbotan.sh
    popd
}

rm -rf install
cmakebuild CDLogger
cmakebuild QueuePtr
botanbuild
cmakebuild cppssh
./build.py
tar jxvf ./build/ComBomb-v*.tar.bz2 -C ../install/
