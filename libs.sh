#!/bin/bash

libf="./libs"
repf="./report"

libs=()

while read -r ln
do
	libs+=("$ln")
done < "$libf"

for lib in "${libs[@]}"; do
#	echo "grep $lib $libf"
#	echo "$(grep "$lib" "$libf")"
	plib="lib${lib}"
	
	if [[ $(grep "$plib:" "$repf") != "" ]]; then
#		echo "GREP"
		continue;
	fi
	
	echo "$plib"

	echo "$plib:" >> $repf

	echo -e "\tStatic:" >> $repf
	for ln in $(timeout 3 sudo find /usr/lib -iname "${plib}.a"); do
		echo -e "\t\t$ln" >> $repf
	done

	echo -e "\tDynamic:" >> $repf
	for ln in $(timeout 3 sudo find /usr/lib -iname "${plib}.so"); do
		echo -e "\t\t$ln" >> $repf
	done

#	echo "lib${lib} END"
done
