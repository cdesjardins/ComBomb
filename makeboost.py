#!/usr/bin/env python
import sys, traceback, tarfile, os, platform, shutil, tempfile, errno, re, urllib2
from subprocess import call
boostname = "boost_1_55_0"
boostfile = boostname + ".tar.bz2"
boosturl = "http://downloads.sourceforge.net/project/boost/boost/1.55.0/" + boostfile
boostdir = boostname + "/boost"

def downloadBoost():
    file_name = boosturl.split('/')[-1]
    u = urllib2.urlopen(boosturl)
    f = open(file_name, 'wb')
    meta = u.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "Downloading: %s Bytes: %s" % (file_name, file_size)

    file_size_dl = 0
    block_sz = 8192
    while True:
        buffer = u.read(block_sz)
        if not buffer:
            break

        file_size_dl += len(buffer)
        f.write(buffer)
        status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
        status = status + chr(8)*(len(status)+1)
        print status,

    f.close()

def extractTar():
    print("Extracting " + boostfile + "...")
    file = tarfile.open(boostfile, "r:bz2")
    file.extractall()
    file.close()

def copyJamFile(arch, toolsetsuffix, compilerVersion):
    compiler = arch + toolsetsuffix
    source = ""
    if (platform.system() == "Windows"):
        line = "using msvc ;\n"
    else:
        if (arch == "x86"):
            if (len(compilerVersion) == 0):
                line = "using gcc : x86 : g++ ;\n"
            else:
                line = "using gcc : " + compiler + compilerVersion + " : g++-" + compilerVersion + " ;\n"
        elif (compiler == "arm"):
            line = "using gcc : arm : arm-linux-gnueabihf-g++ ;\n"
        else:
            raise Exception("Inavlid compiler: " + compiler)
    target = "tools/build/v2/user-config.jam"
    appendLineToFile(target, line)
 
def searchFile(target, line):
    ret = False
    searchfile = open(target, "r")
    for l in searchfile:
        if line in l:
            ret = True
            break
    searchfile.close()
    return ret

def appendLineToFile(target, line):
    if (searchFile(target, line) == False):
        f = open (target, "a")
        f.write(line)
        f.close()

def runBootstrap():
    bootstrap = []
    # bootstrap on windows is done as part of runB2Windows becuase it needs the vcvars also
    if (platform.system() != "Windows"):
        bootstrap = ["./bootstrap.sh"]
        call(bootstrap)

def which(filename):
    for path in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(path + "/" + filename):
                return path + "/" + filename
    return None
    
def runB2Linux(arch, toolsetsuffix, compilerVersion, extraArgs):
    copyJamFile(arch, toolsetsuffix, compilerVersion)
    call(["./b2", "link=static", "-j", "8", "stage", "-a", "toolset=gcc-" + arch + toolsetsuffix + compilerVersion] + extraArgs)
    targetDir = "stage/" + arch
    if (len(compilerVersion) > 0):
         targetDir += "-" + compilerVersion
    os.makedirs(targetDir)
    shutil.move("stage/lib", targetDir)

def runB2Windows(extraArgs):
    batfilefd, batfilename = tempfile.mkstemp(suffix=".bat", text=True)
    file = os.fdopen(batfilefd, 'w')
    msvsfound = False
    msvsvars = []
    for ver in range (11, 8, -1):
        envvar = "VS" + str(ver) + "0COMNTOOLS"
        msvsvars.append(envvar)
        if (envvar in os.environ):
            visualStudioInstallDir = os.environ[envvar];
            file.write("call \"" + visualStudioInstallDir + "..\\..\\VC\\vcvarsall.bat\" x86\n")
            file.write("call bootstrap.bat msvc\n")
            cmd = "b2 --toolset=msvc-" + str(ver) + ".0 " + " ".join(extraArgs) + " link=static -j 8 stage --layout=system -a variant="
            file.write(cmd + "release\n")
            file.write("move stage\\lib stage\\release\n")
            file.write(cmd + "debug\n")
            file.write("move stage\\lib stage\\debug\n")
            file.close()
            print batfilename
            call([batfilename])
            msvsfound = True
            break
    if (msvsfound == False):
        print("Unable to find env var for MSVS, tried: ", msvsvars)

def runB2(extraArgs):
    if (platform.system() == "Windows"):
        copyJamFile("x86", "", "")
        runB2Windows(extraArgs)
    else:
        #runB2Linux("x86", "", "4.8.1", ["cxxflags=-fPIC"])
        #runB2Linux("x86", "", "4.6", ["cxxflags=-fPIC"])
        #runB2Linux("x86", "", "4.4", ["cxxflags=-fPIC"])
        runB2Linux("x86", "", "", ["cxxflags=-fPIC"] + extraArgs)
    
def main(argv):
    try:
        if (os.path.exists(boostfile) == False):
            downloadBoost()
        else:
            print("Skip download of boost archive because the " + boostfile + " already exists")
            
        if (os.path.exists(boostdir) == False):
            extractTar()
        else:
            print("Skip extraction of boost archive because the " + boostdir + " directory already exists")
            stagedir = boostdir + "/../stage"
            print("Deleting: " + stagedir)
            if (os.path.exists(stagedir) == True):
                shutil.rmtree(stagedir)
        os.chdir(boostname)
        runBootstrap()
        extraArgs = [
            "--with-system",
            "--with-thread",
            "--with-chrono",
            ]
        runB2(extraArgs)
    except:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback)
    
if __name__ == "__main__":
    main(sys.argv[1:])

#using gcc : x86 : g++ ;
#using gcc : armhf : arm-linux-gnueabihf-g++ ;
#using gcc : arm : arm-linux-gnueabi-g++ ;
#using msvc ;

