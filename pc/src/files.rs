use std::{collections::HashMap, env::current_dir, path::PathBuf, sync::LazyLock};

pub static FILES: LazyLock<HashMap<String, PathBuf>> = LazyLock::new(|| {
    let manifest_dir = PathBuf::from(current_dir().unwrap());
    let web_dir     = manifest_dir.join("web");
    let assets_dir  = manifest_dir.join("pc").join("assets");

    let index_html  = web_dir.join("index.html");
    let user_ld     = assets_dir.join("user.ld");
    let user_o      = assets_dir.join("user.o");
    let user_elf    = assets_dir.join("user.elf");
    let user_bin    = assets_dir.join("user.bin");

    let files = HashMap::from([
        ("manifest_dir".to_string(), manifest_dir),
        ("web_dir".to_string(), web_dir),
        ("assets_dir".to_string(), assets_dir),
        ("index.html".to_string(), index_html),
        ("user.ld".to_string(), user_ld),
        ("user.o".to_string(), user_o),
        ("user.elf".to_string(), user_elf),
        ("user.bin".to_string(), user_bin),
    ]);

    files
});

pub static DRIVERS: LazyLock<HashMap<String, PathBuf>>  = LazyLock::new(|| {
    let dir = FILES["manifest_dir"].clone().join("drivers");

    let mut files = HashMap::new();
    
    if dir.is_dir() {
        files.extend(
            dir.read_dir()
                .unwrap()
                .flatten()
                .map(|entry| {
                    let name = entry.file_name().to_string_lossy().into_owned();
                    (name.clone(), dir.join(name))
                })
        );
    }

    files
});