use std::{io::{Error, ErrorKind}, sync::Arc, time::Duration};
use tokio::{
    fs, io::{AsyncReadExt, AsyncWriteExt, ReadHalf, WriteHalf, split},
    sync::RwLock, time::sleep,
};
use tokio_serial::{DataBits, FlowControl, Parity, SerialPortBuilderExt, SerialStream, StopBits};

use crate::{ArcRw, ArcRwOpt, MCUData, files::FILES};

const KILOBYTE: u16 = 1024;

#[derive(Clone)]
pub struct USART {
    tx: ArcRwOpt<WriteHalf<SerialStream>>,
    rx: ArcRwOpt<ReadHalf<SerialStream>>,
    mcu_data: ArcRw<MCUData>
}
impl USART {
    pub fn new(mcu_data: ArcRw<MCUData>) -> Self {
        Self {
            tx: Arc::new(RwLock::new(None)),
            rx: Arc::new(RwLock::new(None)),
            mcu_data: mcu_data,
        }
    }

    pub async fn connect(&self) {
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
                .open_native_async()
            {
                Ok(stream) => Some(stream),
                Err(_) => {
                    sleep(Duration::from_secs(1)).await;
                    continue;
                }
            };
            break;
        }
        let (rx, tx) = split(port.unwrap());

        println!(
            "successfully connected to serial port {}",
            mcu_data.read().await.name
        );

        *self.tx.write().await = Some(tx);
        *self.rx.write().await = Some(rx);
    }

    pub async fn clear_struct(self: Arc<Self>) {
        *self.tx.write().await = None;
        *self.rx.write().await = None;
    }

    pub async fn sync(&self) {
        let mut temp = [0u8; 1];

        while temp[0] != 0xFE {
            self.tx.write().await.as_mut().unwrap().write_u8(0xFF).await.unwrap();
            self.rx.write().await.as_mut().unwrap().read(&mut temp).await.unwrap();
        }
    }
    
    pub async fn disconnect_watchdog(self: Arc<Self>) {
        let usart = self.clone();
        let mcu_data = usart.mcu_data.clone();

        let name = mcu_data.read().await.name.clone();

        loop {
            mcu_data.clone().write().await.is_active =
                tokio::fs::try_exists(&name).await.unwrap_or(false);
            tokio::time::sleep(Duration::from_secs(1)).await;
        }
    }

    pub async fn sync_write(&self, data: &Vec<u8>) {
        let data_len = data.len() as u32;
        
        self.sync().await;
        for i in 0..4{
            self.tx.write().await.as_mut().unwrap().write_u8((data_len >> (8 * i)) as u8).await.unwrap();
        }
        for el in data.iter() {
            self.tx.write().await.as_mut().unwrap().write_u8(*el).await.unwrap();
        }
    }

    pub async fn sync_read(&self) -> Vec<u8> {
        let mut buf = Vec::new();
        let mut data_len = 0u32;

        self.sync().await;
        
        for i in 0..4{
            data_len |= u32::from(self.rx.write().await.as_mut().unwrap().read_u8().await.unwrap()) << (8 * i);
        }
       
        buf.resize(data_len as usize, 0);
        for i in 0..data_len {
            buf[i as usize] = self.rx.write().await.as_mut().unwrap().read_u8().await.unwrap();
        }
    
        buf
    }

    pub async fn send_code(&self) -> std::io::Result<u32> {
        if !self.mcu_data.read().await.is_active{
            return Err(Error::from(ErrorKind::HostUnreachable));
        }
        
        let buf = fs::read(FILES["user.bin"].clone()).await?;
        if buf.len() as u16 > KILOBYTE * 8 {
            Err(Error::from(ErrorKind::OutOfMemory))
        } else {
            println!("sending code to MCU");

            self.sync_write(&buf).await;
            Ok(
                self.sync_read().await[0]as u32,
            )
        }
    }
}
