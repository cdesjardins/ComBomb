####http://blog.chrisd.info/combomb/

ComBomb is a terminal emulator for debugging embedded systems in the modern era.

#### How to build on both Windows and Linux

Note: The makeboost python script assumes pyton 2.6 or 2.7, it will download and build boost the way ComBomb expects it.
```
repo init -u git@github.com:cdesjardins/ComBombManifest.git
repo sync
cd build
[./]makeboost.py
[./]makebotan.py
[./]build.py
```

Download Qt source package from: http://www.qt.io/download-open-source/#section-2
I also typically link to Qt5 statically, to build statically I use the following commands:

```
cd qt-everywhere-opensource-src-<version>
```

Windows:
```
configure -opensource -nomake examples -nomake tests -prefix C:\Qt\<version> -confirm-license -static -static-runtime -no-openssl -opengl desktop
jom -j 5
jom install
jom -j 15 distclean
```

Linux:
```
sudo apt-get install libfontconfig1-dev
./configure -opensource -nomake examples -nomake tests -prefix ~/Qt/<version> -confirm-license -static -no-openssl -no-gtkstyle -qt-xcb
make -j5
make install -j
make -j15 distclean
```

Requires:
python 2.6, or 2.7
git must be in the path (for version number generation)

ComBomb uses the following components:

Icons from Free FatCow-Farm Fresh Icons
http://www.fatcow.com/free-icons

cppssh - Small C++11 SSH2 library
https://github.com/cdesjardins/cppssh

Botan - Crypto and TLS for C++11 http://botan.randombit.net
