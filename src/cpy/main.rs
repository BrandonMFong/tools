/**
 * author: Brando
 * date: 7/6/23
 *
 * https://doc.rust-lang.org/rust-by-example/error/result.html
 */

use std::env;
use std::any::type_name;
use std::fs;
use std::path::PathBuf;
use std::fs::canonicalize;

fn main() {
    let mut error = 0;
    let args: Vec<String> = env::args().collect();
    if args.len() > 2 {
        let source = &args[args.len() - 2];
        let dest = &args[args.len() - 1];
        error = copy_from_source_to_destination(source, dest);
    }

    std::process::exit(error);
}

fn copy_from_source_to_destination(s: &String, d: &String) -> i32 {
    let mut flows: Vec<FileFlow> = Vec::new();
    let full_dest_path = canonicalize(d).unwrap().into_os_string().into_string().unwrap();

    match find_leaf_files(s) {
        Err(e) => {
            eprintln!("Experienced an error in: {} - {}", type_name::<fn()>(), e.kind()); 
            return -1;
        } Ok(files) => {
            for file in files {
                flows.push(FileFlow::new(&file, &full_dest_path));
            }
        }
    }

    for flow in flows {
        println!("{} => {}", flow.source, flow.destination);
    }

    return 0;
}

fn find_leaf_files(path: &str) -> Result<Vec<String>, std::io::Error> {
    let mut result = Vec::new();
    let entries = fs::read_dir(path)?; // Read directory entries
    
    for entry in entries {
        let entry = entry?;
        let file_type = entry.file_type()?;
        
        if file_type.is_file() {
            if let Some(file_name) = entry.file_name().to_str() {
                let base_path = PathBuf::from(path);
                let rel_path = base_path.join(file_name);
                let expanded_path = canonicalize(rel_path).unwrap().into_os_string().into_string().unwrap();
                result.push(expanded_path.to_owned());
            }
        } else if file_type.is_dir() {
            let subdir_files = find_leaf_files(entry.path().to_str().unwrap())?;
            result.extend(subdir_files);
        }
    }
    
    Ok(result)
}

struct FileFlow {
    pub source: String,
    pub destination: String
}

impl FileFlow {
    fn new(s: &String, d: &String) -> Self {
        FileFlow {
            source: s.to_string(), destination: d.to_string()
        }
    }
}

