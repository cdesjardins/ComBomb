#!/usr/bin/env python
# Use this script with ComBomb to pipe data from the terminal to a file.
#
# Open the Run Process dialog, and navigate to this script and run it.
#

import getopt, os, sys, datetime

def usage():
    print("-o, --outfile - Name of the output file")
    os._exit(1)

def pipeToFile(filename):
    sys.stderr.write(filename)
    f = open(filename, 'w')
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        f.write(line)
        f.flush()
    f.close()

def main(argv):
    opts, args = getopt.getopt(argv, "ho:", ["help", "outfile="])
    filename = "ComBomb_" + datetime.datetime.now().strftime("%Y%m%d%H%M%S") + ".txt"
    for opt, arg in opts:
        if (opt in ('-h', '--help')):
            usage()
        if (opt in ('-o', '--outfile')):
            filename = arg
    pipeToFile(filename)

if __name__ == "__main__":
    main(sys.argv[1:])
