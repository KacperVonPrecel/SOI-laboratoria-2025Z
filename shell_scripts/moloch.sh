#!/bin/bash

mkdir "projekt"
cd projekt || exit
touch start.txt
mv start.txt koniec.txt
cd ..
ls -l projekt
