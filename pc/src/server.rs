use std::sync::Arc;
use tokio::{task::JoinHandle, sync::RwLock};
use warp::reply::json;
use warp::{path, reply::html, serve, Filter};

use crate::data::Errors;
use crate::usart::USART;
use crate::{ArcRw, Code, MCUData};
use crate::files::FILES;


pub struct Server{
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>,
    code_errors: ArcRw<Errors>,
    html_data: ArcRw<String>,
    handlers: ArcRw<Vec<JoinHandle<()>>>
}
impl Server{
    pub fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self {
        let html_data = std::fs::read_to_string(FILES["index.html"].clone()).unwrap();

        Self {
            mcu_data:   mcu_data, 
            user_code:  user_code,
            code_errors: Arc::new(RwLock::new(Errors::new())),
            html_data:   Arc::new(RwLock::new(html_data)),
            handlers:    Arc::new(RwLock::new(Vec::new()))
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

                move |code: Code| {
                    let server = server.clone();
                    let usart = usart.clone();
                    
                    async move {
                        *server.user_code.write().await = code.clone();

                        let errors = Arc::new(RwLock::new(Errors::new()));
                        *errors.write().await = server.user_code.write().await.compile().await;
                        *server.code_errors.write().await = errors.read().await.clone();
                        
                        if errors.read().await.is_success{
                            let usart = usart.clone();
                            let errors = errors.clone();

                            errors.write().await.returned_value = match usart.send_code().await{
                                Ok(val) => val,
                                Err(_) => 0
                            };
                        }

                        Ok::<_, warp::Rejection>(json(&*errors.read().await))
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
