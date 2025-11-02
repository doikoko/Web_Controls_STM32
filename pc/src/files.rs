use std::{collections::HashMap, env::current_dir, path::PathBuf, sync::LazyLock};

pub static FILES: LazyLock<HashMap<&str, PathBuf>> = LazyLock::new(|| {
    let manifest_dir = PathBuf::from(current_dir().unwrap());
    let web_dir     = manifest_dir.join("web");
    let assets_dir  = manifest_dir.join("pc").join("assets");

    let index_html  = web_dir.join("index.html");
    let user_ld     = assets_dir.join("user.ld");
    let user_o      = assets_dir.join("user.o");
    let user_elf    = assets_dir.join("user.elf");

    let files = HashMap::from([
        ("manifest_dir", manifest_dir),
        ("web_dir", web_dir),
        ("assets_dir", assets_dir),
        ("index.html", index_html),
        ("user.ld", user_ld),
        ("user.o", user_o),
        ("user.elf", user_elf)
    ]);

    files
});