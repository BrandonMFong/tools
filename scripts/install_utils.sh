#
# author: brando
# date: 4/2/24
#
# used by install and uninstall script

## CONSTANTS

ARG_HELP="help";
ARG_HELP_V2="--help";
ARG_HELP_V3="-h";

TARGET_PATH="/usr/local/bin";

# update if there are more things we need to copy over
COMPONENTS=(check cpy getcount getinfo getpath getsize ip4domain listtools mytime organize passgen rsatool search spellcheck stopwatch);

## GLOBALS
gShowHelp=false;

# 
# arguments_read()
#
# returns error code
#
# install and uninstall don't take any args
function arguments_read() 
{
	for arg in "${V_ARGS[@]}"
	do
		if [ "$ARG_HELP" == "$arg" ]; then
			gShowHelp=true;
		elif [ "$ARG_HELP_V2" == "$arg" ]; then
			gShowHelp=true;
		elif [ "$ARG_HELP_V3" == "$arg" ]; then
			gShowHelp=true;
		fi
	done

	return 0;
}

