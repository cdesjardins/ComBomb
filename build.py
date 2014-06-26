#!/usr/bin/env python

import shutil, sys, os, platform, createVersion, zipfile, tarfile
from subprocess import call

def rmerror(function, path, excinfo):
    exc_type, exc_value, exc_traceback = excinfo
    print exc_value
    os._exit(1)

def delBuildTree(delDir):
    retries = 0
    while (os.path.exists(delDir) == True):
        shutil.rmtree(delDir, False, rmerror)
        retries += 1
        if (retries > 10):
            break

def which(file):
    if (platform.system() == "Windows"):
        file += ".exe"
    for path in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    return None

def zipItWindows(filename, qtDir):
    files = {
        "ComBombGui/release/ComBombGui.exe": "ComBombGui.exe",
        qtDir + "/Qt5Widgets.dll": "Qt5Widgets.dll",
        qtDir + "/Qt5Gui.dll": "Qt5Gui.dll",
        qtDir + "/libGLESv2.dll": "libGLESv2.dll",
        qtDir + "/libEGL.dll": "libEGL.dll",
        qtDir + "/Qt5Core.dll": "Qt5Core.dll",
        qtDir + "/icuin51.dll": "icuin51.dll",
        qtDir + "/icudt51.dll": "icudt51.dll",
        qtDir + "/icuuc51.dll": "icuuc51.dll",
        qtDir + "/../plugins/platforms/qminimal.dll": "platforms/qminimal.dll",
        qtDir + "/../plugins/platforms/qwindows.dll": "platforms/qwindows.dll",
    }
    filename += ".zip"
    combombZip = zipfile.ZipFile(filename, "w")
    for k, v in files.iteritems():
        combombZip.write(k, v, zipfile.ZIP_DEFLATED)
    
def zipItPosix(filename, qtDir):
    files = {
        "ComBombGui/ComBombGui": "ComBomb/ComBombGui",
        qtDir + "/../lib/libQt5Widgets.so.5": "ComBomb/libQt5Widgets.so.5",
        qtDir + "/../lib/libQt5Core.so.5": "ComBomb/libQt5Core.so.5",
        qtDir + "/../lib/libQt5Gui.so.5": "ComBomb/libQt5Gui.so.5",
        qtDir + "/../lib/libicui18n.so.51": "ComBomb/libicui18n.so.51",
    }
    filename += ".tar.bz2"
    file = tarfile.open(filename, "w:bz2")
    for k, v in files.iteritems():
        file.add(os.path.realpath(k), v)

def zipIt(gitVerStr, qtDir):
    vers = gitVerStr.split("-")
    filename = "ComBomb-" + vers[0] + "-" + vers[1]
    if (platform.system() == "Windows"):
        zipItWindows(filename, qtDir)
    else:
        zipItPosix(filename, qtDir)
    
def main(argv):
    delBuildTree("build")
    os.makedirs("build")
    os.chdir("build")
    CreateVer = createVersion.CreateVer(sys.argv[1:])
    gitVerStr = CreateVer.run()
    if (gitVerStr.find("dirty") > 0):
        print("Building on dirty codebase: " + gitVerStr)
        #os._exit(1)
    qmake = which("qmake")
    if (qmake == None):
        print("Qmake not found")
        os._exit(1)
    (qtDir, tail) = os.path.split(qmake)
    call([qmake, ".."])
    if (platform.system() == "Windows"):
        call(["nmake"])
        pass
    else:
        call(["make", "-j"])
        pass
    zipIt(gitVerStr, qtDir)
    
    print("Done")

if __name__ == "__main__":
    main(sys.argv[1:])
