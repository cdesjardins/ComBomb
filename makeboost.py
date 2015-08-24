#!/usr/bin/env python
import sys, traceback, tarfile, os, platform, shutil, tempfile, errno, re, urllib2
from subprocess import call
boostname = "boost_1_59_0"
boostfile = boostname + ".tar.bz2"
boosturl = "http://downloads.sourceforge.net/project/boost/boost/1.59.0/" + boostfile
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

def runBootstrap():
    bootstrap = []
    # bootstrap on windows is done as part of runB2Windows becuase it needs the vcvars also
    if (platform.system() != "Windows"):
        bootstrap = ["./bootstrap.sh"]
        call(bootstrap)

def runB2Linux(extraArgs):
    cmd = ["./b2", "link=static", "-j", "8", "stage", "-a", "toolset=gcc"]
    cmd.extend(extraArgs)
    call(cmd)

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
            cmd = "b2 --toolset=msvc-" + str(ver) + ".0 " + " ".join(extraArgs) + " link=static runtime-link=static -j 8 stage --layout=system -a variant="
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
        runB2Windows(extraArgs)
    else:
        extraArgs.extend(["cxxflags=-fPIC"]);
        runB2Linux(extraArgs)
    
def main(argv):
    try:
        if (os.path.exists("../boost") == False):
            os.mkdir("../boost")
        os.chdir("../boost")
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
            "--with-filesystem",
            ]
        runB2(extraArgs)
    except:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback)
    
if __name__ == "__main__":
    main(sys.argv[1:])

