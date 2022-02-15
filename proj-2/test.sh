#!/bin/bash

rm -f huge_file.dat

echo Creating huge text file to compress
for i in {1..1000}
do
    cat test_data.txt >> huge_file.dat
done
echo Done
echo
echo Performing compression
time ./zstd_custom huge_file.dat comp_file.zst
echo Done
echo
echo Decompressing
unzstd comp_file.zst -f -o uncomp_file.dat
echo Done
echo
echo Performing diff
diff huge_file.dat uncomp_file.dat
echo Done
