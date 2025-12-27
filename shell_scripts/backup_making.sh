#!/bin/bash

echo "Give a file to backup:"
read -r file

if [ -f "$file" ]; then
    cp "$file" "$file.bak"
    echo "File exists. Backup was made: $file.bak"
else
    date > "$file"
    echo "File doesn't exist. Writing current date into the file."
fi
