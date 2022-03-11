#!/bin/bash
dirs='examples/Decode examples/Encode examples/Incoming examples/GSM7Check'
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
	cd $d
	cd lib
	if [ -d pdulib ]
	then
		rm -r pdulib
	fi
	if [[ $crdirs = true ]]
	then
		echo "Creating symlinks"
		mkdir pdulib
		cd pdulib
		ln -s ../../../../src/pdulib.cpp pdulib.cpp
		ln -s ../../../../src/pdulib.h pdulib.h
	else
		echo "Not creating symlinks"
	fi
done
echo 'All done'

