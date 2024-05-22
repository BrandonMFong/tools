#!/bin/bash

tools=( \
	check cpy getcount getinfo \
	getpath getsize ip4domain mytime \
	organize passgen rsatool search \
	spellcheck stopwatch define \
);

for tool in "${tools[@]}"
do
	printf "$tool: ";
	cmd="$tool --brief-description";
	eval $cmd;
done

