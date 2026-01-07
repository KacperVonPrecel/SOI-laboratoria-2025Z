#!/bin/sh

echo "Compilation..."
cc fs.c -o fs
if [ $? -ne 0 ]; then echo "Error compiling fs.c"; exit 1; fi

cc generate.c -o gen
if [ $? -ne 0 ]; then echo "Error compiling generate.c"; exit 1; fi

echo "==========================================="
echo "TESTING SCENARIO FOR FILE SYSTEM"
echo "==========================================="

# 1. Creation of the environment
echo "\n[1] Creating virtual disk..."
./fs -c

# Generating test files
# 1 block = 1024 bytes
echo "Generating support files..."

# File A: 10 KB (10 blocks)
./gen fileA 10240

# File B: 20 KB (20 blocks) -> POPRAWKA: 20 * 1024 = 20480
./gen fileB 20480

# File C: 10 KB (10 blocks)
./gen fileC 10240

# File D: Huge file 25 KB (25 blocks)
./gen fileD 25600

# File E: Very small file (5 bytes) - test of internal fragmentation
echo "Hello" > fileE

sleep 2

# 2. Saving files
echo "\n[2] Saving files A, B, C onto virtual disk..."
./fs -cp fileA plik_A
./fs -cp fileB plik_B
./fs -cp fileC plik_C
./fs -cp fileE maly_plik

./fs -map

sleep 3

# 3. Fragmentation demonstration
echo "\n[3] Removing middle file (B) to create a 'hole'..."
./fs -rm plik_B
./fs -map

sleep 3

echo "\n[4] Trying to save file D (25KB) in hole left after B (20KB)..."
echo "    Expected result: Failure (even though total space is enough, the hole is too small)."
./fs -cp fileD plik_D

sleep 3

echo "\n[5] Demonstrating internal fragmentation..."
echo "    File 'maly_plik' has only a few bytes, but takes up whole 1024B block."
./fs -map

sleep 3

# 4. Copying back
echo "\n[6] Copying 'plik_A' from virtual disk to local (as 'odzyskany_A')..."
./fs -out plik_A odzyskany_A

ls -l fileA odzyskany_A

sleep 2

# Cleaning up
echo "\n[7] Cleaning up..."
./fs -rmdisk
rm fs gen fileA fileB fileC fileD fileE odzyskany_A
echo "Test completed."

