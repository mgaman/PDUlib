#!/bin/bash
dirs='examples/Decode examples/Encode examples/Incoming'
current=$PWD
for d in $dirs
do
	#echo $d
	#echo $current
	cd $current
	cd $d
	cd lib
	rm -r pdulib
#	mkdir pdulib
#	cd pdulib
#	ln -s ../../../../src/pdulib.cpp pdulib.cpp
#	ln -s ../../../../src/pdulib.h pdulib.h
done
echo 'All done'

