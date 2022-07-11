#!/bin/bash
dirs='Encode MultiPartSend'
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
	cp DesktopExample/src/credentials.h examples/$d/src  # platformIO
	cp DesktopExample/src/credentials.h examples/$d      # Arduino
done
echo 'All done'

