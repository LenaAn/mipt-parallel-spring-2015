#!/usr/bin/env python

import random
import sys

if __name__ == "__main__":
    # number of equations
    n = int(sys.argv[1]) if len(sys.argv) == 2 else 2000

    sys.stdout = file( "input.txt", "w+" ) 

    # print number of equations
    print n

    # generate and print matrix A
    for i in range(n):
        print " ".join("%8.3lf" % random.uniform(1, 100) for i in range(n))

    # generate and print vector b
    print " ".join("%8.3lf" % random.uniform(1, 100) for i in range(n))

