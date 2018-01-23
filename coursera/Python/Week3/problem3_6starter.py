# -problem3_6.py *- coding: utf-8 -*-

import sys

# open as a read only
infile = open(sys.argv[1])
outfile = open(sys.argv[2], 'w')

for row in infile:
    tmp = row.strip("\n")
    length = len(tmp)
    outfile.write(str(length)+"\n")
    
infile.close()
outfile.close()
# add your code here