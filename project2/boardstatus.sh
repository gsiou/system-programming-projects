#!/bin/bash

# Check if we got a parameter
if [ "$#" -ne 1 ]; then
    echo "Please specify path.";
    exit -1;
fi

echo "-----------BOARD SERVER STATUS----------";
echo "";

boardCount=0;
inactive=0;
active=0;
for dir in $1/*/
do
    if [ -f $dir/server.pid ];
    then
	pid=$(cat $dir/server.pid);
	result=$(ps $pid | grep board-server);
	if [ "$result" = "" ];
	then
	    echo "* Inactive Board: $dir";
	    inactive=$((inactive+1));
	else
	    echo "* Active Board: $dir";
	    active=$((active+1));
	fi
	boardCount=$((boardCount+1));
    fi
done

echo "---------------------------------";
echo "Total active boards: $active";
echo "Total inactive boards: $inactive";
echo "---------------------------------";
