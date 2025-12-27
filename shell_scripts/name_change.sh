#!/bin/bash

if [ $# != 2 ]; then
    echo "Usage: name_change.sh <old_name, new_name>"
    exit 1
fi

if [ -f "$1" ]; then
    mv "$1" "$2"
    echo "Name changing completed"
else
    echo "File does not exist!"
    exit 1
fi
