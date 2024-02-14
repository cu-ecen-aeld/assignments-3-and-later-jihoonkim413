writefile=$1
writestr=$2

# check if the number of the arguments are 2
if [ $# -ne 2 ]
then 
	echo "ERROR: Invalid number of argument"
    exit 1
fi

# check if the first arguemnt is a file
if [ -d $writefile ]
then
    echo "ERROR: Invalid file "
    exit 1
fi

mkdir -p $(dirname $writefile)
if [ -z $? ]
then
    echo "ERROR: cannot create a new path"
fi

echo $writestr >> $writefile


