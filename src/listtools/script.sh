#!/bin/bash

tools=("check" "cpy" "getcount");
for tool in "${tools[@]}"
do
	printf "$tool: ";
	cmd="$tool --brief-description";
	eval $cmd;
done

