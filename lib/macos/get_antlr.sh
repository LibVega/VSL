#!/bin/bash
#
# Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
# This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
# the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
#

# This is used to dynamically download the MacOS Antlr 4 static library, since it is nearly 100MB.

DOWNLOAD_LINK="https://www.antlr.org/download/antlr4-cpp-runtime-4.9.1-macos.zip"
OUTPUT_FILE="antlr.zip"

if [ -f "libantlr4-runtime.a" ]; then 
    echo "Skipping ANTLR download"
    exit 0
fi

curl -s ${DOWNLOAD_LINK} -o ${OUTPUT_FILE} &&\
unzip -j ${OUTPUT_FILE} "lib/libantlr4-runtime.a" -d "." &&\
rm ${OUTPUT_FILE}
