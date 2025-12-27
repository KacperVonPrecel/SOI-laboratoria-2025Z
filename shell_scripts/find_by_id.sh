#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Użycie: $0 <uid>" >&2
    exit 1
fi

given_uid=$1

while IFS=':' read -r user _ uid _ _ _ _; do
    if [ "$uid" = "$given_uid" ]; then
        echo "$user"
        exit 0
    fi
done < /etc/passwd

echo "Nie znaleziono użytkownika o ID $given_uid" >&2
exit 1