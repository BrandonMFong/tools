#!/bin/bash

result=0;
COMMAND=$1;
INPUT_KEY=$2;
INPUT_ITEM=$3;
SCRIPT_NAME=$(basename "$0");
ENCRYPT_ARG="encrypt";
DECRYPT_ARG="decrypt";
GENKEYS_ARG="genkeys";
PUB2PEM_ARG="pub2pem";

function Help() {
	printf "usage: $SCRIPT_NAME <cmd>\n";

	printf "commands: \n";
	printf "\t$ENCRYPT_ARG <public key> [ <input file> | <input string> ]\n";
	printf "\t$DECRYPT_ARG <private key> <input file>\n";
	printf "\t$GENKEYS_ARG - Generates rsa keys\n";
	printf "\t$PUB2PEM_ARG <public key> - Converts public key created by 'ssh-keygen' to 'openssl' format (which is what we need)\n";
}

function Encrypt() {
	if [ $result -eq 0 ]; then
		if [[ ! -f $INPUT_ITEM ]]; then 
			echo $INPUT_ITEM | openssl rsautl -encrypt -inkey $INPUT_KEY -pubin -out out.enc;
		else 
			openssl rsautl -encrypt -inkey $INPUT_KEY -pubin -in $INPUT_ITEM -out $INPUT_ITEM.enc;
		fi

		result=$?;
	fi 
}

function Decrypt() {
	if [ $result -eq 0 ]; then
		openssl rsautl -decrypt -inkey $INPUT_KEY -in $INPUT_ITEM -out $INPUT_ITEM.dec;
		result=$?;
	fi
}

function GenerateKeys() {
	if [ $result -eq 0 ]; then 
		openssl genrsa -out private.key 1024;
		result=$?;
	fi

	if [ $result -eq 0 ]; then 	
		openssl rsa -in private.key -pubout -out public.key; 
		result=$?;
	fi

}

function ConvertPub2Pem() {
	if [ $result -eq 0 ]; then 
		which ssh-keygen >/dev/null 2>&1;

		if [ $? -ne 0 ]; then 
			printf "ssh-keygen does not exist!\n";
			result=1;
		fi 
	fi

	if [ $result -eq 0 ]; then 
		ssh-keygen -f $INPUT_KEY -m 'PEM' -e > "$(basename $INPUT_KEY)".pem
	fi 
}

function main() {
	which openssl >/dev/null 2>&1;

	if [ $? -ne 0 ]; then 
		result=1;
		printf "Openssl does not exist on your system\n";
	fi 

	# If openssl exists, then we will continue with the process!
	if [ $result -eq 0 ]; then 
		if [[ $COMMAND == "$ENCRYPT_ARG" ]]; then
		       Encrypt;	
		elif [[ $COMMAND == "$DECRYPT_ARG" ]]; then
		       Decrypt;	
		elif [[ $COMMAND == "$GENKEYS_ARG" ]]; then 
			GenerateKeys;
		elif [[ $COMMAND == "$PUB2PEM_ARG" ]]; then 
			ConvertPub2Pem;
		else 
			Help;
		fi
	fi 
}

main;

exit $result;
