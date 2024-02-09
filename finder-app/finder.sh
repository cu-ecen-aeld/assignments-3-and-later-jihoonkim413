#!/bin/sh

pwd=$0
filedir=$1
searchstr=$2

# check if the number of the arguments are 2
if [ $# -ne 2 ]
then 
	echo "ERROR: Invalid number of argument"
    exit 1
fi

# check if the second arguement is a directory
if [ ! -d $filedir ]
then
    echo "ERROR: Invalid directory"
    exit 1
fi

cd $filedir
X=$(ls | wc -l) # number of files in the directory and all subdirectories
Y=$(grep -r $searchstr * | wc -l) # the number of matching lines found in respective files, where a matching line refers to a line which contains searchstr (and may also contain additional content)

echo "The number of files are $X and the number of matching lines are $Y"


