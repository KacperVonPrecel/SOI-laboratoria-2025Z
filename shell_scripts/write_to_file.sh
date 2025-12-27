#!/bin/bash

line=""
> test.txt


while read -r line; do
    if [ "$line" = "quit" ]; then
        break
    fi

    echo "$line" >> test.txt
done
