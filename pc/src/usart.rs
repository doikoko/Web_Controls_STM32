use tokio_serial::{DataBits, FlowControl, Parity, SerialPortBuilderExt, SerialStream, StopBits};
use tokio::{io::{split, AsyncReadExt, AsyncWriteExt, ReadHalf, WriteHalf}, sync::RwLock, time::sleep};
use std::{sync::Arc, time::Duration};

use crate::{ArcRwOpt, ArcRw, MCUData, Code};

#[derive(Clone)]
pub struct USART{
    tx: ArcRwOpt<WriteHalf<SerialStream>>,
    rx: ArcRwOpt<ReadHalf<SerialStream>>,
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>
}
impl USART{
    pub fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self{
        Self {
            tx: Arc::new(RwLock::new(None)),
	        rx: Arc::new(RwLock::new(None)),
	        mcu_data: mcu_data,
	        user_code: user_code 
        }
    }

    pub async fn connect(self: Arc<Self>){      
        let mcu_data = self.mcu_data.clone();
        
        let mut port = None;
        loop {
            let name = mcu_data.read().await.name.clone();
            port = match tokio_serial::new(&name, 9600)
                    .timeout(Duration::from_millis(100))
                    .data_bits(DataBits::Eight)
                    .flow_control(FlowControl::None)
                    .parity(Parity::None)
                    .stop_bits(StopBits::One)
                    .open_native_async(){
                        Ok(stream) => {
                            Some(stream)
                        },
                        Err(_) => { 
                            sleep(Duration::from_secs(1)).await;
                            continue;
                        }
                    };
                break;
            }
        let (rx, tx) = split(port.unwrap());
        
        println!("successfully connected to serial port {}", mcu_data.read().await.name);
        
        *self.tx.write().await = Some(tx);
        *self.rx.write().await = Some(rx);
    }

    pub async fn clear_struct(self: Arc<Self>){
        *self.tx.write().await = None;
        *self.rx.write().await = None;
    }

    pub async fn sync(self: Arc<Self>){
        let mut temp = [0u8; 1];

        while temp[0] != 0xFF{
            self.tx.write().await.as_mut().unwrap().write_u8(0xFF).await.unwrap();
            self.rx.write().await.as_mut().unwrap().read(&mut temp).await.unwrap();
        }
    }

    pub async fn disconnect_watchdog(self: Arc<Self>) {
        let usart = self.clone();
        let mcu_data = usart.mcu_data.clone();
        
        let name = mcu_data.read().await.name.clone();

        loop {
            mcu_data.clone().write().await.is_active = tokio::fs::try_exists(&name).await.unwrap_or(false);
            tokio::time::sleep(Duration::from_secs(1)).await;
        }
    }
    pub async fn sync_write(self: Arc<Self>, data: &Vec<u8>){
        self.clone().sync().await;

        for el in data.iter(){
            self.tx.write().await.as_mut().unwrap().write_u8(*el).await.unwrap();
        }
    }
    pub async fn sync_read(self: Arc<Self>) -> Vec<u8> {
        let mut buf = Vec::new();
        let mut temp = [1u8; 1];
        
        self.clone().sync().await;

        while temp[0] != b'\0'{
            temp = [0u8; 1];
            self.rx.write().await.as_mut().unwrap().read(&mut temp).await.unwrap();
            buf.push(temp[0]);
        }
        
        buf
    }
    pub async fn send_code(self: Arc<Self>, code: Arc<Code>){
        //code.compile();
        //self.tx.write().await.write_all(&*code.compiled.unwrap().as_slice());
    }
}

