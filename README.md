####http://blog.chrisd.info/combomb/

ComBomb is a terminal emulator for debugging embedded systems in the modern era.

#### How to build on both Windows and Linux

Note: The makeboost python script assumes pyton 2.6 or 2.7, it will download and build boost the way ComBomb expects it.
```
git clone https://github.com/cdesjardins/ComBomb.git
cd ComBomb
git submodule init
git submodule update
[./]makeboost.py
cd cl
make -j or build with msvc on windows
```

Once those commands are complete build the cryptlib library in the cl directory, and then open ComBomb.pro in QT Creator Based on QT 5, then run qmake, and build.

I also typically link to Qt5 statically, to build a statically I use the following commands:

Windows:
```
perl init-repo --no-webkit
configure -opensource -nomake examples -nomake tests -prefix C:\Qt\<version> -confirm-license -static -no-openssl -opengl desktop
jom -j 5
jom install
jom -j 15 distclean
```

Linux:
```
perl init-repo --no-webkit
./configure -opensource -nomake examples -nomake tests -prefix ~/Qt/<version> -confirm-license -static -no-openssl -no-gtkstyle
make -j5
make install
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
