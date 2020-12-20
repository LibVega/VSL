#!/bin/bash
#
# Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
# This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
# the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
#

# This is the Unix shell script for generating the premake project.

IsMac=0
IsLinux=0
if [ "$(uname)" = "Darwin" ]; then
	IsMac=1
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
	IsLinux=1
elif [ "$(expr substr $(uname -s) 1 5)" = "MINGW" ]; then
	echo "ERROR: MinGW is not a supported build system for Polaris"
	exit 1
elif [ "$(expr substr $(uname -s) 1 6)" = "CYGWIN" ]; then
	echo "ERROR: Cygwin is not a supported build system for Polaris"
	exit 1
else
	echo "ERROR: Build platform not understood: $(uname)"
	exit 1
fi

if [ $IsMac -eq 1 ]; then
	./premake/premake5_m --file=./polaris.project gmake2
elif [ $IsLinux -eq 1 ]; then
	./premake/premake5_l --file=./polaris.project gmake2
fi
