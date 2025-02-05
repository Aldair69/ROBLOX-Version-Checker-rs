use std::collections::HashMap;
use std::env;
use std::fs;
use std::path::Path;

// Struct to hold verion info (date and tipe)
#[derive(Debug)]
struct VersionInfo {
    date: String,
    version_type: String,
}

// Function to read the embbeded file (simulated here with external file)
fn read_embbeded_file() -> Result<String, String> {
    // Simulating embedded file using normal file read.
    // In a real exe, you could use include_bytes! to embbed the content.
    let file_content = fs::read_to_string("deployhistory.txt")
        .map_err(|_| "Couldnt read the embbeded deployhistory.txt file".to_string())?;
    Ok(file_content)
}

// Function to extract hashes and verion info from deployhistory.txt content
fn extract_hashes(deploy_history: &str) -> HashMap<String, VersionInfo> {
    let mut hashes = HashMap::new();

    for line in deploy_history.lines() {
        let parts: Vec<&str> = line.split(',').collect();
        if parts.len() == 3 {
            let hash = parts[0].trim_start_matches("version-").to_string();
            let date = parts[1].to_string();
            let version_type = parts[2].to_string();
            hashes.insert(hash, VersionInfo { date, version_type });
        }
    }

    hashes
}

// Function to serch installed verions in Roblox directory
fn search_verions(hashes: &HashMap<String, VersionInfo>) {
    // Get LOCALAPPDATA envirenment varible
    let local_app_data = env::var("LOCALAPPDATA").unwrap_or_else(|_| {
        eprintln!("Couldnt retrive LOCALAPPDATA environment variable");
        String::new()
    });

    if local_app_data.is_empty() {
        return;
    }

    // Build path to Roblox/Versions directory
    let versions_dir = Path::new(&local_app_data).join("Roblox").join("Versions");

    if !versions_dir.exists() {
        eprintln!(
            "Versions directory dont exists at {}. Maybe Roblox isnt installed?",
            versions_dir.display()
        );
        return;
    }

    // Iterate over folders in Versions dir
    if let Ok(entries) = fs::read_dir(&versions_dir) {
        for entry in entries.filter_map(Result::ok) {
            if let Some(folder_name) = entry.file_name().to_str() {
                if folder_name.starts_with("version-") {
                    let hash = folder_name.trim_start_matches("version-").to_string();
                    if let Some(version_info) = hashes.get(&hash) {
                        println!(
                            "Found version hash: {} | Date: {} | Type: {}",
                            hash, version_info.date, version_info.version_type
                        );
                    }
                }
            }
        }
    } else {
        eprintln!("Couldnt read versions directory: {}", versions_dir.display());
    }
}

fn main() {
    println!("lordarathres2's Version Locater in Rust");

    // Read the embbeded deployhistory.txt file
    let deploy_history = match read_embbeded_file() {
        Ok(content) => content,
        Err(error) => {
            eprintln!("{}", error);
            return;
        }
    };

    // Extract hashes and verion info
    let hashes = extract_hashes(&deploy_history);

    // Serch for installed verions that match the hashes
    search_verions(&hashes);
}
