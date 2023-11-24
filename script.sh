#!/bin/bash 

if test "$#" -ne 1
then exit
fi

count=0

while read line
do
if  echo "$line" | grep -E "^[A-Z][A-Za-z0-9 ,]*" | grep -E "$1" | grep -v ",[ ]*si " | grep -v " si,[ ]*" |  grep -q -E "[?|!|.]$"
then (( count++ ))
fi
done

echo "$count"