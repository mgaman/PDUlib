#!/bin/bash
dirs='Decode Encode Incoming GSM7Check MultiPartSend'
if [ $# -gt 0 ]
then
	crdirs=false
else
	crdirs=true
fi
#echo $crdirs
current=$PWD
for d in $dirs
do
	cd $current
	cd examples/$d
	cp src/$d.c++ $d.ino
	cp src/*.h .
done
echo 'All done'

