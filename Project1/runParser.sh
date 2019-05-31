#!/bin/bash
#remove files if exist
#file prep
file="user.dat"
if [ -f $file ] ; then
    rm $file
fi
file="category.dat"
if [ -f $file ] ; then
    rm $file
fi
file="bids.dat"
if [ -f $file ] ; then
    rm $file
fi
file="description.dat"
if [ -f $file ] ; then
    rm $file
fi
file="items.dat"
if [ -f $file ] ; then
    rm $file
fi

python skeleton_parser.py items-*.json

sort -u -o category.dat category.dat
sort -u -o user.dat user.dat
sort -u -o bids.dat bids.dat
sort -u -o items.dat items.dat
sort -u -o description.dat description.dat
