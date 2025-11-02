use std::{io::{BufRead, Error, ErrorKind}, sync::Arc};
use serde::{Deserialize, Serialize};
use tokio::{io::AsyncWriteExt, process::Command};

use crate::files::FILES;

#[derive(Serialize)]
pub struct MCUData{
    pub name: String,
    pub is_active: bool,
    pub temperature: u8,
}
impl MCUData{
    pub fn new() -> Self {
        println!("input serial port path\nExample: /dev/ttyUSB0");

        let mut name = String::new();
        std::io::stdin().lock().read_line(&mut name).unwrap();

        Self {
            name: name.trim().to_string(),
            is_active: false,
            temperature: 0
        }
    }
}

#[derive(Deserialize, Clone)]
pub struct Code{
    pub code: String,
}
impl Code{
    pub fn new()-> Self{
        Self {
            code: String::new(),
        }
    }

    pub async fn compile(&mut self) -> std::io::Result<()>{
        let mut child = Command::new("arm-none-eabi-g++")
            .arg("-x")
            .arg("c++")
            .arg("-c")
            .arg("-o")
            .arg(FILES["user.o"].clone())
            .arg("-mthumb")
            .arg("-O3")
            .arg("-mcpu=cortex-m4")
            .arg("-fdata-sections")
            .arg("-Wall")
            .arg("-Wextra")
            .arg("-mfloat-abi=hard")
            .arg("-mfpu=fpv4-sp-d16")
            .arg("-")
            .stdin(std::process::Stdio::piped())
            .stdout(std::process::Stdio::inherit())
            .stderr(std::process::Stdio::inherit())
            .spawn()?;

        if let Some(mut stdin) = child.stdin.take(){
            stdin.write_all(&self.code.as_bytes()).await?;
            stdin.shutdown().await?;
        }

        let status = child.wait().await?;
        if !status.success(){
            println!("compilation error");
            return Err(Error::from(ErrorKind::InvalidData));
        }

        let status = Command::new("arm-none-eabi-ld")
            .arg(FILES["user.o"].clone())
            .arg("-T")
            .arg(FILES["user.ld"].clone())
            .arg("-o")
            .arg(FILES["user.elf"].clone())
            .stdin(std::process::Stdio::piped())
            .stdout(std::process::Stdio::inherit())
            .stderr(std::process::Stdio::inherit())
            .spawn()?
            .wait()
            .await?;

        if status.success(){
            println!("successfully compilated");
            Ok(())
        } else {
            Err(Error::from(ErrorKind::InvalidData))
        }
    }
}