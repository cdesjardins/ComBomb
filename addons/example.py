#!/usr/bin/env python

import sys

# If do data has been read for 5 seconds then the wait will end
# but so long as there is a constant stream of data without the
# expected prompt in it, then it will continue to wait until it
# gets the expected prompt
def waitForPrompt(prompt):
    timeout = 5
    while True:
        rlist, _, _ = select([sys.stdin], [], [], timeout)
        if rlist:
            line = sys.stdin.readline()
        else:
            break
        if prompt in line:
            break

def main(argv):
    for i in range(1, 10):
        print("ls /usr/lib -1")
        # Make sure the data gets to the remote end
        sys.stdout.flush()
        # Wait for the command to complete
        waitForPrompt("$")

if __name__ == "__main__":
    main(sys.argv[1:])
