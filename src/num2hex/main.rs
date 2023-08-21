//
// author: Brando
// date: 3/2/23
//

extern crate bflib;

use bflib::core::string::string_is_numeric;
use std::env;

const ARGUMENT_HELP: &str = "-h";
const ARGUMENT_BRIEF_DESCRIPTION: &str = "--brief-description";

fn help() {
    println!("usage: {} <value> {}", env::args().nth(0).unwrap(), ARGUMENT_HELP);
}

fn brief_description() {
    println!("converts number to hex");
}

fn main() {
    let mut error: i32 = 0;
    let mut get_help: bool = false;
    let mut input_value: i32 = 0;
    let mut i: i8 = 0;
    let mut briefly_describe: bool = false;

    for argument in env::args() {
        if i > 0 {
            if argument == ARGUMENT_BRIEF_DESCRIPTION {
                briefly_describe = true
            } else if argument == ARGUMENT_HELP {
                get_help = true
            } else {
                if string_is_numeric(argument.clone()) {
                    input_value = argument.parse::<i32>().unwrap();
                } else {
                    error = 1;
                    println!("Error: '{}' is not a number", argument);
                    println!("Error: Please make sure input value is a number, containing no characters");
                    break;
                }
            }
        }

        i += 1;
    }

    if error == 0 {
        if get_help || env::args().count() == 1 {
            help();
        } else if briefly_describe {
            brief_description();
        } else {
            println!("{:#x}", input_value);
        }
    }

}

