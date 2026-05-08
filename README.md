#### https://github.com/cdesjardins/ComBomb

ComBomb is a terminal emulator for debugging embedded systems in the modern era.

#### How to build on both Windows and Linux

Note: All python scripts should work with python 2.6 or higher (including python 3.x)
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

#### Generating SSH key pairs

ComBomb's SSH client loads private keys in PKCS#8 PEM format
(`-----BEGIN PRIVATE KEY-----`). Use `ssh-keygen` with `-m PKCS8` to
produce a compatible key file for any of the supported algorithms.

RSA:
```
ssh-keygen -t rsa -b 4096 -m PKCS8 -f ~/.ssh/id_rsa
```

ECDSA (NIST P-256, P-384, P-521):
```
ssh-keygen -t ecdsa -b 256 -m PKCS8 -f ~/.ssh/id_ecdsa
ssh-keygen -t ecdsa -b 384 -m PKCS8 -f ~/.ssh/id_ecdsa
ssh-keygen -t ecdsa -b 521 -m PKCS8 -f ~/.ssh/id_ecdsa
```

Ed25519:
```
ssh-keygen -t ed25519 -m PKCS8 -f ~/.ssh/id_ed25519
```

If you already have a key in OpenSSH's default format
(`-----BEGIN OPENSSH PRIVATE KEY-----`), convert it in place with:
```
ssh-keygen -p -m PKCS8 -f <keyfile>
```

To convert a PEM/PKCS#8 key back to OpenSSH's default format:
```
ssh-keygen -p -N "" -f <keyfile>
```

Requires:
git must be in the path (for version number generation)

ComBomb uses the following components:

Icons from Free FatCow-Farm Fresh Icons
http://www.fatcow.com/free-icons

cppssh - Small C++11 SSH2 library
https://github.com/cdesjardins/cppssh

Botan - Crypto and TLS for C++11 http://botan.randombit.net
