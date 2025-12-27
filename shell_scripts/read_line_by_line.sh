#!/bin/bash
counter=1

while IFS= read -r line || [ -n "$line" ]; do
    echo "$counter $line"
    counter=$((counter + 1))
done
