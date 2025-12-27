#!/bin/bash

result=""

for arg in "$@"; do
    result="$arg $result"
done

echo "$result"
