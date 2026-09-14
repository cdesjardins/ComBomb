#### https://github.com/cdesjardins/ComBomb

ComBomb is a terminal emulator for debugging embedded systems in the modern era.

#### How to build on both Windows and Linux

The workspace is managed with [west](https://docs.zephyrproject.org/latest/develop/west/index.html)
(`pip install west`). This repository is the west manifest repository: `west.yml`
lists the sibling projects, and `west update` fetches them.

Note: All python scripts should work with python 2.6 or higher (including python 3.x)
```
west init -m ssh://git@github.com/cdesjardins/ComBomb.git combomb
cd combomb
west update
cd build
[./]makeboost.py
[./]makebotan.py
[./]build.py
```

This leaves the workspace laid out as:
```
combomb/
|-- ComBomb/           this repository, the Qt6 GUI
|-- QueuePtr/
|-- cppssh/
|-- CDLogger/
|-- include/
|-- build/             the build scripts
|-- external/          boost and botan
`-- install/           staged libraries and the packaged GUI
```

For later work, `west update` re-syncs the workspace and `[./]build.py` rebuilds.
`makeboost.py` and `makebotan.py` only need rerunning when Boost or Botan change.

The build project also contributes these as west commands — `west cb-boost`,
`west cb-botan` and `west cb-build` — which run from anywhere in the workspace,
plus `west cb-shell` to drop into the Ubuntu 22.04 container, where the same
`./build.py` produces a binary whose glibc floor is low enough to ship.
See `build/README.md`.

Download Qt source package from: http://www.qt.io/download-open-source/#section-2
I also typically link to Qt5 statically, to build statically I use the following commands:

```
cd qt-everywhere-opensource-src-<version>
```

Windows:
```
mkdir -p dev\qt-build
cd dev\qt-build
\path\to\qt-everywhere-src-6.8.3\configure -prefix c:\Qt\6 -opensource -confirm-license -static -static-runtime -c++std c++20 -nomake examples -nomake tests -no-openssl -opengl desktop
cmake --build . --parallel
cmake --install .
```

Linux:
```
mkdir -p ~/dev/qt-build
cd ~/dev/qt-build
/path/to/qt-everywhere-src-6.x.x/configure -prefix ~/Qt/6 -opensource -confirm-license -static -c++std c++20 -nomake examples -nomake tests -no-openssl -no-feature-gtk3 -qt-pcre -system-freetype -fontconfig
cmake --build . --parallel
cmake --install .
```

#### Making a release

Tagging happens once, on one machine. Publishing happens on every platform you
ship from, and never creates a tag.

```
west cb-tag "what changed in this release"
west cb-release
```

`cb-tag` tags every project in the workspace `v<year>.<dayofyear>.<hour>`, pushes
the tags, commits a manifest with every project frozen to a SHA, tags that, and
leaves this repository checked out at the new tag. `cb-release` then builds it —
in the Ubuntu 22.04 container on Linux, natively on Windows — creates the GitHub
release using the tag message as the notes, and uploads the artifact.

On the other platform, check out the same tag and publish into the release that
is already there:

```
git -C ComBomb fetch --tags
git -C ComBomb checkout v2026.257.14
west update
west cb-release
```

That `west update` is what makes both platforms build identical sources: the
manifest committed at the tag pins every project to a SHA, so nothing can drift
between the two machines. The Linux `.tar.bz2` and the Windows `.zip` land side
by side in the one release.

`cb-release` cannot create a tag. It refuses to run unless the version reported
by `createVersion` — the same string that names the archive — is an exact tag,
and it passes `--verify-tag` to `gh`, which aborts rather than pushing a tag that
is not already on the remote. Run it with `-n` to build and check without
publishing anything.

Releases need the GitHub CLI (`gh`) authenticated for this repository; see
`build/README.md` for the script-level detail.

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
west (`pip install west`), and git must be in the path (for version number
generation). Cutting a release also needs the GitHub CLI (`gh`).

ComBomb uses the following components:

Icons from Free FatCow-Farm Fresh Icons
http://www.fatcow.com/free-icons

cppssh - Small C++11 SSH2 library
https://github.com/cdesjardins/cppssh

Botan - Crypto and TLS for C++11 http://botan.randombit.net
