// 
// author: Brando
// date: 3/2/23

use std::env;

const ARGUMENT_HELP: &str = "-h";

fn help() {
    println!("usage: {} <value> {}", env::args().nth(0).unwrap(), ARGUMENT_HELP);

    println!();
    println!("Discussion:");
    println!("  This is used to easily print out the binary representation of an integer");

    println!();
    println!("Arguments:");
    println!("  <value> : Can be a literal integer or hexidecimal.  If it is a hexidecimal");
    println!("            then it must start with '0x'.  At the moment, we only support 32");
    println!("            bit integers.  Anything higher will have truncated bits in output");
    println!();
}

fn string_is_numeric(str: String) -> bool {
    for c in str.chars() {
        if !c.is_numeric() {
            return false;
        }
    }
    return true;
}

fn main() {
    let mut error: i32 = 0;
    let mut get_help: bool = false;
    let mut input_value: i32 = 0;
    let mut i: i8 = 0;

    for argument in env::args() {
        if i > 0 {
            if argument == ARGUMENT_HELP {
                get_help = true
            } else {
                if argument.starts_with("0x") {
                    let wo_pref = argument.trim_start_matches("0x");
                    input_value = i32::from_str_radix(wo_pref, 16).ok().unwrap();
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
        }

        i += 1;
    }

    if error == 0 {
        if get_help || env::args().count() == 1 {
            help();
        } else {
            let e: u32 = input_value.ilog2();
            if e <= 4 {
                println!("{:#06b}", input_value);
            } else if e <= 8 {
                println!("{:#010b}", input_value);
            } else if e <= 12 {
                println!("{:#014b}", input_value);
            } else if e <= 16 {
                println!("{:#018b}", input_value);
            } else if e <= 20 {
                println!("{:#022b}", input_value);
            } else if e <= 24 {
                println!("{:#026b}", input_value);
            } else if e <= 28 {
                println!("{:#030b}", input_value);
            } else if e <= 32 {
                println!("{:#034b}", input_value);
            }
        }
    }
}

