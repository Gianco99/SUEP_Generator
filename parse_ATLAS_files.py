#!/usr/bin/python
import sys
import os
from glob import glob
import numpy as np


datadir="/home/data1/knapen/softbomb/data/ATLAS_checks/"

filelist=glob(datadir+"*.hepmc")

print filelist

for filename in filelist:
    print filename
    fin = open(filename,"r")
    fout = open(filename+".txt","w")
    for line in fin:
        line=line.split(" ")
        if line[0]=="E":
            evennr=line[1]
        if line[0]=="P" and (line[2]=="13" or line[2]=="-13"):
            fout.write(evennr+" "+line[2]+" "+line[3]+" "+line[4]+" "+line[5]+"\n")
    fout.close()


