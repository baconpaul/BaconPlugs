#!/bin/bash

cat <<- EOH
# Automatically generated BaconMusic release

This release is automatically generated every time I push BaconMusic
to master. As such it is the latest version of the code and may be
unstable, unusable, unsuitable for human consumption, and so on.
The stable release is always available in the Rack Plugin Manager.

These assets were built against
https://vcvrack.com/downloads/Rack-SDK-1.0.0.zip

The build date and most recent commits are:

EOH
date
echo ""
echo "Most recent commits:" 
echo ""
git log --pretty=oneline | head -5
