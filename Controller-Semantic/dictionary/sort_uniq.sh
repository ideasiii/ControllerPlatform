#!/bin/sh

echo 'Backup File'
cp $1 $1.bk

echo 'Sort and Uniq'
sort -r $1 | uniq > $1.new
rm -f $1
mv $1.new $1

echo 'Finish Thanks'
