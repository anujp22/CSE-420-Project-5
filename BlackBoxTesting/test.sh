#!/bin/sh
# This is a comment

rm *.txt
make clean
make

./mydisksim input_file/in.txt student_out_fcfs_1.txt fcfs 1
./mydisksim input_file/in.txt student_out_fcfs_10.txt fcfs 10
./mydisksim input_file/in.txt student_out_fcfs_100.txt fcfs 100
./mydisksim input_file/in.txt student_out_fcfs.txt fcfs
./mydisksim input_file/in.txt student_out_sstf_1.txt sstf 1
./mydisksim input_file/in.txt student_out_sstf_10.txt sstf 10
./mydisksim input_file/in.txt student_out_sstf_100.txt sstf 100
./mydisksim input_file/in.txt student_out_sstf.txt sstf
./mydisksim input_file/in.txt student_out_scan_1.txt scan 1
./mydisksim input_file/in.txt student_out_scan_10.txt scan 10
./mydisksim input_file/in.txt student_out_scan_100.txt scan 100
./mydisksim input_file/in.txt student_out_scan.txt scan
./mydisksim input_file/in.txt student_out_clook_1.txt clook 1
./mydisksim input_file/in.txt student_out_clook_10.txt clook 10
./mydisksim input_file/in.txt student_out_clook_100.txt clook 100
./mydisksim input_file/in.txt student_out_clook.txt clook

if diff -w student_out_fcfs_1.txt correct_output/correct_out_fcfs_1.txt; then
    echo Test 01 - Success--------------------FCFS-1-----------------------------------Success
else
    echo Test 01 - Fail-----------------------FCFS-1-----------------------------------Fail
fi

if diff -w student_out_fcfs_10.txt correct_output/correct_out_fcfs_10.txt; then
    echo Test 02 - Success--------------------FCFS-10----------------------------------Success
else
    echo Test 02 - Fail-----------------------FCFS-10----------------------------------Fail
fi

if diff -w student_out_fcfs_100.txt correct_output/correct_out_fcfs_100.txt; then
    echo Test 03 - Success--------------------FCFS-100---------------------------------Success
else
    echo Test 03 - Fail-----------------------FCFS-100---------------------------------Fail
fi

if diff -w student_out_fcfs.txt correct_output/correct_out_fcfs.txt; then
    echo Test 04 - Success--------------------FCFS-ALL---------------------------------Success
else
    echo Test 04 - Fail-----------------------FCFS-ALL---------------------------------Fail
fi



if diff -w student_out_sstf_1.txt correct_output/correct_out_sstf_1.txt; then
    echo Test 05 - Success--------------------SSTF-1-----------------------------------Success
else
    echo Test 05 - Fail-----------------------SSTF-1-----------------------------------Fail
fi

if diff -w student_out_sstf_10.txt correct_output/correct_out_sstf_10.txt; then
    echo Test 06 - Success--------------------SSTF-10----------------------------------Success
else
    echo Test 06 - Fail-----------------------SSTF-10----------------------------------Fail
fi

if diff -w student_out_sstf_100.txt correct_output/correct_out_sstf_100.txt; then
    echo Test 07 - Success--------------------SSTF-100---------------------------------Success
else
    echo Test 07 - Fail-----------------------SSTF-100---------------------------------Fail
fi

if diff -w student_out_sstf.txt correct_output/correct_out_sstf.txt; then
    echo Test 08 - Success--------------------SSTF-ALL---------------------------------Success
else
    echo Test 08 - Fail-----------------------SSTF-ALL---------------------------------Fail
fi



if diff -w student_out_scan_1.txt correct_output/correct_out_scan_1.txt; then
    echo Test 09 - Success--------------------SCAN-1-----------------------------------Success
else
    echo Test 09 - Fail-----------------------SCAN-1-----------------------------------Fail
fi

if diff -w student_out_scan_10.txt correct_output/correct_out_scan_10.txt; then
    echo Test 10 - Success--------------------SCAN-10----------------------------------Success
else
    echo Test 10 - Fail-----------------------SCAN-10----------------------------------Fail
fi

if diff -w student_out_scan_100.txt correct_output/correct_out_scan_100.txt; then
    echo Test 11 - Success--------------------SCAN-100---------------------------------Success
else
    echo Test 11 - Fail-----------------------SCAN-100---------------------------------Fail
fi

if diff -w student_out_scan.txt correct_output/correct_out_scan.txt; then
    echo Test 12 - Success--------------------SCAN-ALL---------------------------------Success
else
    echo Test 12 - Fail-----------------------SCAN-ALL---------------------------------Fail
fi



if diff -w student_out_clook_1.txt correct_output/correct_out_clook_1.txt; then
    echo Test 13 - Success--------------------CLOOK-1----------------------------------Success
else
    echo Test 13 - Fail-----------------------CLOOK-1----------------------------------Fail
fi

if diff -w student_out_clook_10.txt correct_output/correct_out_clook_10.txt; then
    echo Test 14 - Success--------------------CLOOK-10---------------------------------Success
else
    echo Test 14 - Fail-----------------------CLOOK-10---------------------------------Fail
fi

if diff -w student_out_clook_100.txt correct_output/correct_out_clook_100.txt; then
    echo Test 15 - Success--------------------CLOOK-100--------------------------------Success
else
    echo Test 15 - Fail-----------------------CLOOK-100--------------------------------Fail
fi

if diff -w student_out_clook.txt correct_output/correct_out_clook.txt; then
    echo Test 16 - Success--------------------CLOOK-ALL--------------------------------Success
else
    echo Test 16 - Fail-----------------------CLOOK-ALL--------------------------------Fail
fi
