use std::io::{BufRead, Error, ErrorKind};
use serde::{Deserialize, Serialize};
use tokio::{io::AsyncWriteExt, process::Command};

use crate::files::FILES;

#[derive(Serialize, Clone)]
pub struct Errors{
    pub is_success: bool,
    data: String
}
impl Errors{
    pub fn new() -> Self{
        Self{
            is_success: false,
            data: String::new()
        }
    }
    
    fn push_err(&mut self, error: String){
        if self.is_success{
            self.is_success = false;
        }

        if !error.is_empty(){
            self.data = format!("{}\n{}", self.data, error);
        }
    }

    fn set_success(&mut self){
        self.is_success = true;
        self.data = String::from("success");
    }
}

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

    pub async fn compile(&mut self) -> Errors{
        self.code = self.code.replace("\"driver.hpp\"", "\"drivers/driver.hpp\"")
            .replace("#include \"drivers/code.hpp\"", "\0");

        let mut errors = Errors::new();

        let mut child = Command::new("arm-none-eabi-g++")
            .arg("-x")
            .arg("c++")
            .arg("-c")
            .arg("-o")
            .arg(FILES["user.o"].clone())
            .arg("-mthumb")
            .arg("-O3")
            .arg("-mcpu=cortex-m4")
            .arg("-Wall")
            .arg("-Wextra")
            .arg("-mfloat-abi=hard")
            .arg("-emain")
            .arg("-mfpu=fpv4-sp-d16")
            .arg("-")
            .stdin(std::process::Stdio::piped())
            .stdout(std::process::Stdio::piped())
            .stderr(std::process::Stdio::piped())
            .spawn().unwrap();

        if let Some(mut stdin) = child.stdin.take(){
            stdin.write_all(&self.code.as_bytes()).await.unwrap();
            stdin.shutdown().await.unwrap();
        }

        let output = child.wait_with_output().await.unwrap();
        if !output.status.success(){
            println!("compilation error");

            errors.push_err(String::from_utf8(output.stdout).unwrap());
            errors.push_err(String::from_utf8(output.stderr).unwrap());
            
            return errors;
        }

        let output = Command::new("arm-none-eabi-ld")
            .arg(FILES["user.o"].clone())
            .arg("-T")
            .arg(FILES["user.ld"].clone())
            .arg("-o")
            .arg(FILES["user.elf"].clone())
            .arg("-emain")
            .output()
            .await.unwrap();

        if !output.status.success(){
            println!("linking error");

            errors.push_err(String::from_utf8(output.stdout).unwrap());
            errors.push_err(String::from_utf8(output.stderr).unwrap());

            return errors;
        }

        let output = Command::new("arm-none-eabi-objcopy")
            .arg("-O")
            .arg("binary")
            .arg(FILES["user.elf"].clone())
            .arg(FILES["user.bin"].clone())
            .output()
            .await.unwrap();

        if !output.status.success(){
            println!("converting to binary error");
            
            errors.push_err(String::from_utf8(output.stdout).unwrap());
            errors.push_err(String::from_utf8(output.stderr).unwrap());
        } else {
            println!("successfully compilated");

            errors.set_success();
        }
        
        errors
    }
}
