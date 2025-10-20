use std::{env::current_dir, fs, path::PathBuf, sync::{Arc, RwLock}, time::Duration};
use tokio_serial::{DataBits, FlowControl, Parity, SerialPortBuilderExt, StopBits};
use tokio::{io::split, time::sleep};
use warp::{path, reply::{html, json}, serve, Filter};
use serde::{Serialize, Deserialize};

#[derive(Serialize)]
struct MCUData{
    is_active: bool,
    temperature: u8,
}

#[derive(Deserialize)]
struct Code(String);

#[tokio::main]
async fn main() -> std::io::Result<()>{
    let mut name = String::new();
    
    println!("input path to serial port\nExample: /dev/ttyUSB0");
    std::io::stdin().read_line(&mut name)?;
    
    name = name.trim().to_string();
    
    let manifest_dir = PathBuf::from(current_dir()?);
    let web_dir = manifest_dir.join("web");
    let index_html = web_dir.join("index.html");
    
    
    let html_data: &'static str  = Box::leak(fs::read_to_string(index_html).unwrap().into_boxed_str());
    
    let mcu_data = Arc::new(
        RwLock::new(
            MCUData{
                is_active: false, temperature: 50
            }
        )
    );
    tokio::join!(
        async {
            let mcu_data = mcu_data.clone();
            let mut port = None;
            while !mcu_data.read().unwrap().is_active{
                port = match tokio_serial::new(&name, 115200) 
                    .timeout(Duration::from_millis(100))
                    .data_bits(DataBits::Eight)
                    .flow_control(FlowControl::None)
                    .parity(Parity::Even)
                    .stop_bits(StopBits::One)
                    .open_native_async(){
                        Ok(stream) => {
                            mcu_data.write().unwrap().is_active = true;
                            Some(stream)
                        },
                        Err(_) => { 
                            sleep(Duration::from_secs(1)).await;
                            continue; 
                        }
                };
            }
            let (mut rx, mut tx) = split(port.unwrap());
        
            println!("successfully connected to serial port {}", &name);
        },
        async {
            let mcu_data = mcu_data.clone();
            let rout = path::end()
                .map(move || { 
                        html(html_data)
                    });
                let data = path("data")
                    .and(warp::get())
                    .map(move || {
                        json(&*mcu_data.read().unwrap())
                    });
                    
                    let routers = rout
                    .or(data)
                    .or(warp::fs::dir(web_dir));
                
                println!("your server is ready: http://localhost:8080");
                serve(routers).run(([127, 0, 0, 1], 8080)).await;    
        }
    );

    Ok(())
}
