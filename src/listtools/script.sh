#!/bin/bash

tools=(check cpy getcount getpath getsize);
for tool in "${tools[@]}"
do
	printf "$tool: ";
	cmd="$tool --brief-description";
	eval $cmd;
done

