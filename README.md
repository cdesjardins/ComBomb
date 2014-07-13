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
make -j or build is msvc on windows
```

Once those commands are complete build the cryptlib library in the cl directory, and then open ComBomb.pro in QT Creator Based on QT 5, then run qmake, and build.

Requires:
python 2.6, or 2.7
git must be in the path (for version number generation)


