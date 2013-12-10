#!/usr/bin/env python
import sys, platform, shutil, os.path, tarfile, zipfile
from subprocess import Popen, PIPE

linuxSrcExeFile = "../build-ComBomb-Desktop_Qt_5_1_1_GCC_64bit-Release/ComBombGui/ComBombGui"
linuxDstExeFile = os.path.expanduser("~/Dropbox/ComBomb/lin/")
windowsDstExeFile = os.path.expanduser("~/Dropbox/ComBomb/win/")

def copyExeFile():
    if (platform.system() == "Windows"):
        pass
    elif (platform.system() == "Linux"):
        shutil.copy(linuxSrcExeFile, linuxDstExeFile)
    else:
        print("Unsupported platform")
        exit(1)
        
def zipper(dir, zip_file):
    zip = zipfile.ZipFile(zip_file, 'w', compression=zipfile.ZIP_DEFLATED)
    root_len = len(os.path.abspath(dir))
    for root, dirs, files in os.walk(dir):
        archive_root = os.path.abspath(root)[root_len:]
        for f in files:
            fullpath = os.path.join(root, f)
            archive_name = os.path.join(archive_root, f)
            zip.write(fullpath, archive_name, zipfile.ZIP_DEFLATED)
    zip.close()
    return zip_file

def compressFilesWindows(version):
    print version
    filename = "ComBomb-" + version + ".zip"
    zipper(windowsDstExeFile, filename)
    return filename

def compressFilesLinux(version):
    filename = "ComBomb-" + version + ".tar.gz"
    file = tarfile.open(filename, "w:gz")
    file.add(linuxDstExeFile, arcname="ComBomb-" + version)
    return filename
    
def getVersion():
    process = Popen(["git", "describe", "--dirty", "--always"], stdout=PIPE)
    gitVerStr = process.communicate()[0].strip()
    if ("dirty" in gitVerStr):
        print("Code base is dirty " + gitVerStr)
        gitVerStr = gitVerStr[0:gitVerStr.rfind("-")]
    gitVerStr = gitVerStr[0:gitVerStr.rfind("-")]
    return gitVerStr

def compressFiles(version):
    filename = ""
    if (platform.system() == "Windows"):
        filename = compressFilesWindows(version)
    elif (platform.system() == "Linux"):
        filename = compressFilesLinux(version)
    else:
        print("Unsupported platform")
        exit(1)
    return filename

def main(argv):
    copyExeFile()
    version = getVersion()
    compressFiles(version)

if __name__ == "__main__":
    main(sys.argv[1:])

