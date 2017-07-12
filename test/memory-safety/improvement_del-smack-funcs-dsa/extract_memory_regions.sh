#!/bin/bash

#command used to run: ./extract_memory_regions.sh "$(ls  | grep -E *\\.bpl)"
while IFS=' ' read -ra FILES; do
	for var in "${FILES}"
	do
		name="${var%.*}"
		str=`cat $name.bpl | grep "Memory maps"`
		echo $name --\> ${str:16:1} >> stats
	done
done <<< "$@"
