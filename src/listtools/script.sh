#!/bin/bash

tools=(check cpy getcount getpath getsize ip4domain mytime num2bin num2hex);
for tool in "${tools[@]}"
do
	printf "$tool: ";
	cmd="$tool --brief-description";
	eval $cmd;
done

