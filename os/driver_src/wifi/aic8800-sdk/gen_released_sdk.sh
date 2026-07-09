#!/bin/sh

base_name=`git describe --dirty`
ptch_name=`git describe --tags --dirty`
date_time=`git log -1 --pretty=format:"%cd" --date=short`
if [ -z ${base_name#*-dirty} ]; then
    echo "Dirty branch detected, commit firstly"
    exit 1
fi
if [ ${base_name} == ${ptch_name} ]; then
    file_name="${base_name}-${date_time}"
else
    file_name="${base_name%base*}patch${ptch_name#sdk}-${date_time}"
fi
out_dir=$1
out_file=${out_dir/%\//}/${file_name}.tar.gz
git archive --format=tar --verbose HEAD -- ./ | gzip > ${out_file}
echo ""
echo "Generate sdk file: " ${out_file}
