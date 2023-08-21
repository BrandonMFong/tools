//
// author: Brando
// date: 2/16/23
// 

use std::env;
use std::str;
use std::path::Path;
use std::{thread, time};

const ARGUMENTS_HELP: &str = "-h";
const BRIEF_DESCRIPTION_HELP: &str = "--brief-description";

fn help(args: Vec<String>) {
    let tool_path = Path::new(&args[0]);

    println!("usage: {} [ {} ]", tool_path.file_name().unwrap().to_str().unwrap(), ARGUMENTS_HELP);

    print!("\n");
    println!("Discussion:");
    println!("  This tool will simply print out current elapsed time since the");
    println!("  start of execution.  The format will be MM:SS:mmm.  To record");
    println!("  laps, simply press enter.  To cancel, ctrl+c.");
}

fn brief_description() {
    println!("simple stop watch");
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.contains(&ARGUMENTS_HELP.to_string()) {
        help(args);
    } else if args.contains(&BRIEF_DESCRIPTION_HELP.to_string()) {
        brief_description();
    } else {
        let ten_millis = time::Duration::from_millis(10);
        let now = time::Instant::now();

        loop {
            print!("{:02}:{:02}.{:03}", now.elapsed().as_secs() / 60, now.elapsed().as_secs() % 60, now.elapsed().as_millis());
            thread::sleep(ten_millis);
            print!("\r");
        }
    }
}

