#!/bin/sh
echo "Cleaning out SVN directories..."
find . | grep .svn | tac | awk '{print "rm -rf "$0}' | sh

echo "Making distribution tarball from SVN directory..."
cd ../
cat gtkwave3/src/version.h | grep WAVE_VERSION_INFO | sed 's/.*Analyzer v//' | sed 's/ .*//' | \
	awk '{print "mv gtkwave3 gtkwave-"$0" ; tar cvf gtkwave-"$0".tar gtkwave-"$0" ; gzip -9 gtkwave-"$0".tar"}' | sh

echo "Done!"
