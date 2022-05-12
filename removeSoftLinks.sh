#!/bin/bash
#
#  Run this before submitting to Arduino library
#  Symbolic links not allowed there
#
dirs='examples/Decode examples/Encode examples/Incoming examples/GSM7Check examples/MultiPartSend'
current=$PWD
for d in $dirs
do
	cd $current
	cd $d/lib
	rm -r pdulib
	#echo $PWD
done
echo 'All done'

