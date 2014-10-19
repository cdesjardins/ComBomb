#!/usr/bin/env python

import shutil, sys, os, platform, createVersion, zipfile, tarfile
from subprocess import call
from subprocess import Popen, PIPE

releaseNotes = "releasenotes.txt"

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
    print(file + " not found")
    os._exit(1)
    return None

def zipItWindows(filename, qtDir):
    msvs10Dir = os.environ['VS100COMNTOOLS'] + "../../VC/redist/x86/Microsoft.VC100.CRT/"
    files = {
        "ComBombGui/release/ComBombGui.exe": "ComBombGui.exe",
        qtDir + "/Qt5Widgets.dll": "Qt5Widgets.dll",
        qtDir + "/Qt5Gui.dll": "Qt5Gui.dll",
        qtDir + "/Qt5Core.dll": "Qt5Core.dll",
        qtDir + "/icuin52.dll": "icuin52.dll",
        qtDir + "/icudt52.dll": "icudt52.dll",
        qtDir + "/icuuc52.dll": "icuuc52.dll",
        qtDir + "/../plugins/platforms/qminimal.dll": "platforms/qminimal.dll",
        qtDir + "/../plugins/platforms/qwindows.dll": "platforms/qwindows.dll",
        msvs10Dir + "msvcr100.dll": "msvcr100.dll",
        msvs10Dir + "msvcp100.dll": "msvcp100.dll",
        releaseNotes : releaseNotes,
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
        qtDir + "/../lib/libQt5DBus.so.5": "ComBomb/libQt5DBus.so.5",
        qtDir + "/../lib/libicui18n.so.52": "ComBomb/libicui18n.so.52",
        qtDir + "/../lib/libicuuc.so.52": "ComBomb/libicuuc.so.52",
        qtDir + "/../lib/libicudata.so.52": "ComBomb/libicudata.so.52",
        qtDir + "/../plugins/platforms/libqxcb.so": "ComBomb/platforms/libqxcb.so",
        releaseNotes : "ComBomb/" + releaseNotes,
    }
    filename += ".tar.bz2"
    file = tarfile.open(filename, "w:bz2")
    for k, v in files.iteritems():
        file.add(os.path.realpath(k), v)

def zipIt(gitVerStr, qtDir):
    vers = gitVerStr.split("-")

    filename = "ComBomb-" + vers[0]
    if (len(vers) > 1):
        filename = filename + "-" + vers[1]
    if (platform.system() == "Windows"):
        zipItWindows(filename, qtDir)
    else:
        zipItPosix(filename, qtDir)
    
def buildLog():
    logFile = open('releasenotes.txt', 'w')
    process = Popen(["git", "log", "--pretty=%an %ai %d %s"], stdout=logFile)
    process.wait()
    logFile.flush()
    logFile.close()

def main(argv):
    delBuildTree("build")
    os.makedirs("build")
    os.chdir("build")
    CreateVer = createVersion.CreateVer(sys.argv[1:])
    gitVerStr = CreateVer.run()
    if (gitVerStr.find("dirty") > 0):
        raw_input("Building on dirty codebase (" + gitVerStr + "): ")
    qmake = which("qmake")
    (qtDir, tail) = os.path.split(qmake)
    call([qmake, ".."])
    if (platform.system() == "Windows"):
        call([which("jom")])
        pass
    else:
        call(["make", "-j"])
        pass
    buildLog()
    zipIt(gitVerStr, qtDir)
    
    print("Done")

if __name__ == "__main__":
    main(sys.argv[1:])
