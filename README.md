####http://blog.chrisd.info/combomb/

ComBomb is a terminal emulator for debugging embedded systems in the modern era.

#### How to build on both Windows and Linux

Note: The makeboost python script assumes pyton 2.6 or 2.7, it will download and build boost the way ComBomb expects it.
```
repo init -u git@github.com:cdesjardins/ComBombManifest.git
repo sync
cd ComBomb
[./]makeboost.py
[./]build.py
```

Once those commands are complete build the cryptlib library in the cl directory, and then open ComBomb.pro in QT Creator Based on QT 5, then run qmake, and build.

I also typically link to Qt5 statically, to build a statically I use the following commands:
Download Qt source package from: http://www.qt.io/download-open-source/#section-2

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

Cryptlib is used for ssh, Copyright 1992-2010 Peter Gutmann.
http://www.cs.auckland.ac.nz/~pgut001/cryptlib/

Icons from Free FatCow-Farm Fresh Icons
http://www.fatcow.com/free-icons

