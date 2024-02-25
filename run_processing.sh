#!/bin/bash

while read val; do
    ./sce_dx_processor $1 $val "WE"
done < binlist.txt
