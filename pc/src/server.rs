use std::{sync::Arc, path::PathBuf};
use tokio::{task::JoinHandle, sync::RwLock};
use warp::{path, reply::html, serve, Filter};

use crate::usart::USART;
use crate::{ArcRw, Code, MCUData, usart};
use crate::files::FILES;


pub struct Server{
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>,
    html_data: ArcRw<String>,
    handlers: ArcRw<Vec<JoinHandle<()>>>
}
impl Server{
    pub fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self {
        let html_data = std::fs::read_to_string(FILES["index.html"].clone()).unwrap();

        Self {
            mcu_data:   mcu_data, 
            user_code:  user_code,
            html_data:  Arc::new(RwLock::new(html_data)),
            handlers:   Arc::new(RwLock::new(Vec::new()))
        }       
    }

    pub async fn configure(self: Arc<Self>, usart: Arc<USART>){
        let server = self.clone();
        let rout = path::end()
            .and_then({
                let server = server.clone();
                move || { 
                    let server = server.clone();
                    async move {
                        Ok::<_, warp::Rejection>(html((*server.html_data.read().await).clone()))
                    }
                }
            }).boxed();

        let data_send = path("data")
            .and(warp::get())
            .and_then({
                let server = server.clone();
                move || {
                    let server = server.clone();
                    async move {
                        Ok::<_, warp::Rejection>(warp::reply::json(&*server.mcu_data.read().await))    
                    }
                }
            }).boxed();
            
        let data_get = path("data")
            .and(warp::post())
            .and(warp::body::json())
            .and_then({
                let server = server.clone();
                let usart = usart.clone();
                move |mut code: Code| {
                    let server = server.clone();
                    let usart = usart.clone();
                    
                    async move {
                        server.handlers.clone().write().await.push(tokio::spawn(async move{
                           usart.send_code(&mut code).await
                        }));
                        Ok::<_, warp::Rejection>(warp::reply())
                    }
               }
            }).boxed();
            
        let routers = rout
            .or(data_send)
            .or(data_get)
            .or(warp::fs::dir(FILES["web_dir"].clone()));

        println!("your server is ready: http://localhost:8080");
        serve(routers).run(([127, 0, 0, 1], 8080)).await;
    }

    pub async fn abort_handlers(&self){
        for handler in self.handlers.write().await.iter(){
            handler.abort();
        }
    }
}
