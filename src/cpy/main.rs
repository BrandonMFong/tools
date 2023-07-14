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
use std::io::{self, Read, Write};
use std::path::Path;
use std::convert::TryInto;

const BUFFER_SIZE: usize = 8192; // Chunk size for copying
const ARG_HELP: &str = "h";

fn help() {
    let args: Vec<String> = env::args().collect();
    //println!("usage: {} [ -{} ]");
}

fn main() {
    let mut error = 0;
    let args: Vec<String> = env::args().collect();

    // The last two args should be the source and destination path
    // respectively
    if args.len() > 2 {
        let source = &args[args.len() - 2];
        let dest = &args[args.len() - 1];
        error = copy_from_source_to_destination(source, dest);
    } else if args.len() > 1 {
        let flags = &args[1];
        if flags.contains(ARG_HELP) {

        }
    }

    std::process::exit(error);
}

/**
 * Copies all items in s to d
 */
fn copy_from_source_to_destination(s: &String, d: &String) -> i32 {
    // vector of source/destination pairs
    let mut flows: Vec<FileFlow> = Vec::new();

    // Get full paths for params
    let full_source_path = canonicalize(s).unwrap().into_os_string().into_string().unwrap();
    let full_dest_path = canonicalize(d).unwrap().into_os_string().into_string().unwrap();

    // Find all items in source directory
    println!(" - Unfolding sources for all leaf items");
    match find_leaf_files(s) {
        Err(e) => {
            eprintln!("Experienced an error in: {} - {}", type_name::<fn()>(), e.kind()); 
            return -1;
        } Ok(files) => {
            for file in files {
                flows.push(FileFlow::new(&full_source_path, &file, &full_dest_path));
            }
        }
    }

    // Do copy
    // 
    // This will only copy one by one
    println!(" - Initiating copy...");
    for mut flow in flows {
        // make sure we know where we are copying to
        if flow.setup() != 0 {
            return -1;
        }

        // Execute copy
        match flow.copy() {
            Ok(_) => {}
            Err(e) => {
                eprintln!("Error copying file {}: {}", flow.source, e);
                return -1;
            }
        }
    }

    return 0;
}

/**
 * Finds all file paths within path
 */
fn find_leaf_files(path: &str) -> Result<Vec<String>, std::io::Error> {
    let mut result = Vec::new();
    let entries = fs::read_dir(path)?; // Read directory entries
    
    for entry in entries {
        let entry = entry?;
        let file_type = entry.file_type()?;
       
        // if file, save path. otherwise recursively call function
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
    /// Source file
    pub source: String,

    /// destination path
    pub destination: String,

    /// Base path where source is from
    base: String,

    /// Where source file will go respecting
    /// the file structure in base path
    new_destination: String
}

impl FileFlow {

    fn new(b: &String, s: &String, d: &String) -> Self {
        FileFlow {
            base: b.to_string(),
            source: s.to_string(),
            destination: d.to_string(),
            new_destination: String::new()
        }
    }

    /**
     * Returns the relative leaf path from base
     *
     * if base == source, then file_name() of source is returned
     */
    fn source_rel_leaf(&self) -> String {
        let mut result = PathBuf::new();
 
        // if source == base then we can assume:
        //
        // 1: source input is a file, we just need to append file name to destination path
        // 2: source input is an empty directory. This case is not handled yet
        if self.source == self.base {
            let source_path = Path::new(&self.source);
            if let Some(leaf) = source_path.file_name() {
                result.push(leaf);
            }
        } else {
            result.push(Path::new(&self.base).file_name().unwrap());
            
            // strip base from source path
            let mut leaf_rel_path = self.source.replace(&self.base, "");

            // remove the "/" so it is not treated as an absolute path but
            // rather a relative path
            leaf_rel_path = Path::new(&leaf_rel_path).strip_prefix("/").unwrap().display().to_string();

            // We use the leaf component of base to add to the destination path
            // because we want to make sure if the input source to this
            // program is a directory, we make sure we copy from the root of 
            // the source
            result.push(leaf_rel_path);
        }

        return result.into_os_string().into_string().unwrap();
    }

    /// sets newDestination
    pub fn setup(&mut self) -> i32 {
        let mut dest_path = PathBuf::from(&self.destination);
        
        // Create new destination path, keeping the structure of the
        // base path
        dest_path.push(self.source_rel_leaf());

        self.new_destination = dest_path.clone().into_os_string().into_string().unwrap();

        // Make sure sub directories are created
        let dest_parent_path = dest_path.parent().unwrap();
        match fs::create_dir_all(dest_parent_path) {
            Ok(_) => {}
            Err(e) => {
                eprintln!("could not create directory {}: {}", dest_parent_path.display(), e);
                return -1;
            }
        }

        return 0;
    }

    /// Copies source to newDestination
    pub fn copy(&self) -> io::Result<()> {
        let mut source_file = fs::File::open(&self.source)?;
        let source_size: usize = source_file.metadata().unwrap().len().try_into().unwrap();
        let mut destination_file = fs::File::create(&self.new_destination)?;
        let mut buffer = [0; BUFFER_SIZE];
        let mut total_bytes_copied = 0;

        print!(" - {} - {:.2}%", self.source_rel_leaf(), (total_bytes_copied as f64 / source_size as f64) * 100.0);
        loop {
            let bytes_read = source_file.read(&mut buffer)?;
            if bytes_read == 0 {
                break; // End of file
            }

            destination_file.write_all(&buffer[..bytes_read])?;

            total_bytes_copied += bytes_read;

            print!("\r");
            print!(" - {} - {:.2}%", self.source_rel_leaf(), (total_bytes_copied as f64 / source_size as f64) * 100.0);
        }
        println!("\r - {} - {:.2}%", self.source_rel_leaf(), (total_bytes_copied as f64 / source_size as f64) * 100.0);

        Ok(())
    }
}

