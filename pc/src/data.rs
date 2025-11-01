use std::io::BufRead;
use serde::Serialize;
use tokio::process::Command;

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

pub struct Code{
    pub code: String,
    pub compiled: Option<Vec<u8>>
}
impl Code{
    pub fn new()-> Self{
        Self {
            code: String::new(),
            compiled: None
        }
    }

    pub fn from_string(str: String) -> Self{
        Self {
            code: str,
            compiled: None
        }
    }
    
    pub async fn compile(&mut self){
        let _ = Command::new("arm-none-eabi-g++")
            .arg("-x").arg("c++").arg("-Tuser.ld")
            .arg("-ouser.elf").arg("-mthumb").arg("-O3")
            .arg("-mcpu=cortex-m4").arg("-fdata-sections")
            .arg("-Wall").arg("Wextra")
            .arg("mfloat-abi=hard").arg("-mfpu=fpv4-sp-d16")
                .spawn()
                .unwrap()
                .wait()
                .await;
        self.compiled = Some(tokio::fs::read("user.elf").await.unwrap().clone());
    }
}