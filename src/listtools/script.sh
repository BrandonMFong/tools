#!/bin/bash

tools=(check cpy getcount getpath getsize ip4domain mytime num2bin num2hex organize passgen rsatool stopwatch);
for tool in "${tools[@]}"
do
	printf "$tool: ";
	cmd="$tool --brief-description";
	eval $cmd;
done

