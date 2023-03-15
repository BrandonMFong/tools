/**
 * author: brando
 * date: 3/15/23
 */

package main

import (
	"fmt"
	"os"
	"strconv"
)

var kArgumentsHelp = "-h"
var kArgumentsShiftLeft = "l"
var kArgumentsShiftRight = "r"

func help() {
	fmt.Printf("usage %v <num> <shift> [ %v | %v ] [ %v ]\n",
		os.Args[0],
		kArgumentsShiftLeft,
		kArgumentsShiftRight,
		kArgumentsHelp)
	fmt.Println()
	fmt.Printf("  shifts num n times to the direction %v (left) or %v (right), where n is the shift argument\n",
		kArgumentsShiftLeft,
		kArgumentsShiftRight)
}

func SliceContainsValue(slice []string, value string) bool {
	for _, v := range slice {
		if v == value {
			return true
		}
	}

	return false
}

func main() {
	var e error
	var errorcode = 0

	if SliceContainsValue(os.Args, kArgumentsHelp) || len(os.Args) == 1 {
		help();
	} else if len(os.Args) > 4 || len(os.Args) < 3 {
		fmt.Println("Error: incorrect amount of arguments")
		errorcode = 1
	} else {
		var num, shift int
		num, e = strconv.Atoi(os.Args[1])
		if e != nil {
			errorcode = 2
		}

		if errorcode == 0 {
			shift, e = strconv.Atoi(os.Args[2])
			if e != nil {
				errorcode = 3
			}
		}

		var dir string = kArgumentsShiftLeft
		if errorcode == 0 {
			if len(os.Args) > 3 {
				dir = os.Args[3]

				if dir != kArgumentsShiftLeft && dir != kArgumentsShiftRight {
					errorcode = 4
					fmt.Printf("%v is not recognized, please enter either %v or %v\n", dir, kArgumentsShiftLeft, kArgumentsShiftRight);
				}
			}
		}

		if errorcode == 0 {
			if dir == kArgumentsShiftLeft {
				fmt.Println(num << shift)
			} else {
				fmt.Println(num >> shift)
			}
		}
	}

	os.Exit(errorcode)
}

