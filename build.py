#!/usr/bin/env python

# This is the master combomb build script
# which is used to create releases.

import shutil, sys, os, platform, createVersion, zipfile, tarfile, getopt
from subprocess import call
from subprocess import Popen, PIPE

releaseNotes = "releasenotes.txt"
gitVersions = {}

class uncrustify:
    def __init__(self):
        self.home = os.path.expanduser("~")
        self.uncrust = self.home + "/bin/call_Uncrustify.sh"
        self.config  = self.home + "/bin/uncrustify.cfg"

    def callUncrustify(self, directory, ext):
        process = Popen([self.uncrust, directory, ext])
        process.wait()

    def uncrustify(self, directory):
        if (platform.system() == "Linux"):
            if ((os.path.isfile(self.uncrust) == True) and (os.path.isfile(self.config) == True)):
                self.callUncrustify(directory, "cpp")
                self.callUncrustify(directory, "h")
        CreateVer = createVersion.CreateVer()
        gitVerStr = CreateVer.getVerStr()
        if (gitVerStr.find("dirty") > 0):
            raw_input("Building on dirty codebase (" + gitVerStr + " - " + os.getcwd() + "): ")
        return gitVerStr

def cmakeBuildLinux():
    cmakeRelease = "cmake -DCMAKE_BUILD_TYPE=Release .."
    cmakeDebug = "cmake -DCMAKE_BUILD_TYPE=Debug .."
    make = "make -j8 install"
    call(cmakeRelease.split(' '))
    call(make.split(' '))
    delBuildTree(".")
    call(cmakeDebug.split(' '))
    call(make.split(' '))

def cmakeBuildWindows():
    cmake = "cmake .."
    cmakeRelease = "cmake --build . --target install --config Release"
    cmakeDebug = "cmake --build . --target install --config Debug"
    call(cmake.split(' '))
    call(cmakeRelease.split(' '))
    call(cmakeDebug.split(' '))

def cmakeBuild(baseDir):
    os.chdir(baseDir)
    if (delBuildTree("build") == True):
        os.mkdir("build")
    os.chdir("build")
    gitVerStr = uncrustify().uncrustify("..")
    gitVersions[baseDir] = gitVerStr
    if (platform.system() == "Linux"):    
        cmakeBuildLinux()
    else:
        cmakeBuildWindows()
    os.chdir("../..")

def botanBuild():
    os.chdir("cppssh")
    if (platform.system() == "Linux"):    
        call(["./buildbotan.sh"])
    else:
        call(["buildbotan.bat"])
    os.chdir("..")

def handleComBombDirty(gitVerStr):
    for k, v in gitVersions.iteritems():
        if (v != gitVerStr):
            dirty = True
            index = gitVerStr.find("dirty")
            if (index == -1):
                gitVerStr += "-libs"
            else:
                gitVerStr = gitVerStr.replace("dirty", "libs")
            cmd = "git tag -a " + gitVerStr + " -m"
            cmdArray = cmd.split(' ')
            cmdArray.extend(["\"build script says you are dirty\""])
            call(cmdArray)
            gitVerStr += "-dirty"
            break
    return gitVerStr 

def cleanupComBombDirty(gitVerStr):
    gitVerStr = gitVerStr.replace("-dirty", "")
    cmd = "git tag -d " + gitVerStr
    call(cmd.split(' '))

def combombBuild():
    os.chdir("ComBomb")
    gitVerStr = uncrustify().uncrustify(".")
    newGitVerStr = handleComBombDirty(gitVerStr)
    if (delBuildTree("build") == True):
        os.mkdir("build")
    os.chdir("build")
    qmake = which("qmake")
    (qtDir, tail) = os.path.split(qmake)
    call([qmake, ".."])
    if (platform.system() == "Windows"):
        call([which("jom"), "-j", "5", "release"])
    else:
        call(["make", "-j5"])
    buildLog()
    zipIt(newGitVerStr, qtDir)
    if (gitVerStr != newGitVerStr):
        cleanupComBombDirty(newGitVerStr)
    os.chdir("../..")

def delBuildTree(delDir):
    retries = 0
    while (os.path.exists(delDir) == True):
        shutil.rmtree(delDir, True)
        retries += 1
        if (retries > 10):
            break
    return not os.path.exists(delDir)

def which(file):
    if (platform.system() == "Windows"):
        file += ".exe"
    for path in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    print(file + " not found")
    os._exit(1)
    return None

files = {
    "../ComBombGui/images/ComBomb128.png": "ComBomb/ComBomb128.png",
    releaseNotes : "ComBomb/" + releaseNotes,
    "../addons/savetofile.py" : "ComBomb/addons/savetofile.py",
}

def zipItWindows(filename, qtDir):
    files["ComBombGui/release/ComBombGui.exe"] = "ComBomb/ComBombGui.exe"
    filename += ".zip"
    combombZip = zipfile.ZipFile(filename, "w")
    for k, v in files.iteritems():
        combombZip.write(k, v, zipfile.ZIP_DEFLATED)
    
def zipItPosix(filename, qtDir):
    files["ComBombGui/ComBombGui"] = "ComBomb/ComBombGui"
    filename += ".tar.bz2"
    file = tarfile.open(filename, "w:bz2")
    for k, v in files.iteritems():
        file.add(os.path.realpath(k), v)

def zipIt(gitVerStr, qtDir):
    vers = gitVerStr.split("-")

    #filename = "ComBomb-" + vers[0]
    #if (len(vers) > 1):
    #    filename = filename + "-" + vers[1]

    filename = "ComBomb-" + gitVerStr
    if (platform.system() == "Windows"):
        zipItWindows(filename, qtDir)
    else:
        zipItPosix(filename, qtDir)
    latest = open("latest.txt", 'w')
    latest.write(vers[0])
    latest.close()
    
def buildLog():
    logFile = open(releaseNotes, 'w')
    process = Popen(["git", "log", "--pretty=%an %ai %d %s"], stdout=logFile)
    process.wait()
    logFile.flush()
    logFile.close()

def usage(builds):
    print("Build the ComBomb software suite")
    print("The following modules can be individually built")
    for b in builds:
        print ("    --" + b)
    os._exit(1)

def main(argv):
    builds = ["botan", "QueuePtr", "CDLogger", "cppssh", "ComBomb"]
    buildVals = {}
    for b in builds:
        buildVals[b] = True
    args = ["help"]
    args.extend(builds)
    buildsToRun = []
    try:
        opts, args = getopt.getopt(argv, "h", args)
    except getopt.GetoptError as e:
        print "Error: " + str(e)
        usage(builds)
    for opt, arg in opts:
        if (opt in ('-h', '--help')):
            usage(builds)
        if opt[2:] in buildVals.keys():
            buildsToRun.append(opt[2:])

    os.chdir("..")
    if (len(buildsToRun) > 0):
        for b in builds:
            buildVals[b] = False
        for b in buildsToRun:
            buildVals[b] = True
    else:
        delBuildTree("install")
    if (buildVals["botan"] == True):
        botanBuild()
    if (buildVals["QueuePtr"] == True):
        cmakeBuild("QueuePtr")
    if (buildVals["CDLogger"] == True):
        cmakeBuild("CDLogger")
    if (buildVals["cppssh"] == True):
        cmakeBuild("cppssh")
    if (buildVals["ComBomb"] == True):
        combombBuild()
    print("Done")

if __name__ == "__main__":
    main(sys.argv[1:])
