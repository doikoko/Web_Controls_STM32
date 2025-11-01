use std::{sync::Arc, time::Duration};
use tokio::{sync::RwLock};

mod data;
mod server;
mod usart;

use crate::data::*;
use crate::server::*;
use crate::usart::*;

type ArcRw<T> = Arc<RwLock<T>>;
type ArcRwOpt<T> = Arc<RwLock<Option<T>>>;

#[tokio::main]
async fn main() -> std::io::Result<()>{
    let mcu_data = Arc::new(RwLock::new(MCUData::new()));
    let user_code = Arc::new(RwLock::new(Code::new()));

    let usart = Arc::new(USART::new(mcu_data.clone(), user_code.clone()));
    let server = Arc::new(Server::new(mcu_data.clone(), user_code.clone()));

    let usart_watchdog_handler = tokio::spawn(
        async move {
            tokio::spawn(usart.clone().disconnect_watchdog());

            let mut is_active = false;
            
            loop{
                let usart = usart.clone();
                if !mcu_data.clone().read().await.is_active{
                    if is_active {
                        println!("serial port {} is disconnected", mcu_data.read().await.name);
                        usart.clear_struct().await;
                    }
                    is_active = false;
                }
                else if mcu_data.read().await.is_active{
                    if !is_active{
                        usart.connect().await;
                    }
                    is_active = true;
                }
                
                tokio::time::sleep(Duration::from_secs(1)).await;
            }
        }
    );
    
    let mut server_handler = Some(tokio::spawn(server.clone().configure()));

    tokio::select!(
        _ = server_handler.take().unwrap() => { usart_watchdog_handler.abort(); },
        _ = tokio::signal::ctrl_c() => {
            println!("shutting down...");
            
            server.wait_handlers().await;

            usart_watchdog_handler.abort();
            server_handler.take().unwrap().abort();
        }
    );

    Ok(())
}
