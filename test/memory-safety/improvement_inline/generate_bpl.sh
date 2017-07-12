##!/bin/bash

#ran with command ./generate_bpl.sh "$(ls .. | grep -E *\\.c)"
#the file name is prepend with ".." so run the command in appropriate directory 

while IFS=' ' read -ra FILES; do
	for var in "${FILES}"
	do
		name="${var%.*}"
		#`echo $name.bpl`
		echo running $name
		smack --memory-safety ../$var -bpl $name.bpl
	done
done <<< "$@"
