use std::{sync::Arc, path::PathBuf, env::current_dir};
use tokio::{task::JoinHandle, sync::RwLock};
use warp::{path, reply::html, serve, Filter};

use crate::{ArcRw, Code, MCUData};

struct ServerFiles{
    web_dir: PathBuf,
    index_html: PathBuf
}
pub struct Server{
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>,
    files: ArcRw<ServerFiles>,
    html_data: ArcRw<String>,
    handlers: ArcRw<Vec<JoinHandle<()>>>
}
impl Server{
    pub fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self {
        let manifest_dir = PathBuf::from(current_dir().unwrap());
        let web_dir = manifest_dir.join("web");
        let index_html = web_dir.join("index.html");

        let files = ServerFiles {
            web_dir:        web_dir,
            index_html:     index_html,
        };

        let html_data = std::fs::read_to_string(&files.index_html).unwrap();

        Self {
            mcu_data:   mcu_data, 
            user_code:  user_code,
            files:      Arc::new(RwLock::new(files)),
            html_data:  Arc::new(RwLock::new(html_data)),
            handlers:   Arc::new(RwLock::new(Vec::new()))
        }       
    }

    pub async fn configure(self: Arc<Self>){
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
                move |code: String| {
                    let server = server.clone();
                    async move {
                        //(*server).user_code.write().await.code = code.clone();
                        //
                        //server.handlers.write().await.push(tokio::spawn(async move{
                        //    usart.send_code(Code::from_string(code)).await
                        //}));
                        Ok::<_, warp::Rejection>(warp::reply())
                    }
               }
            }).boxed();
            
        let routers = rout
            .or(data_send)
            .or(data_get)
            .or(warp::fs::dir(server.clone().files.read().await.web_dir.clone()));

        println!("your server is ready: http://localhost:8080");
        serve(routers).run(([127, 0, 0, 1], 8080)).await;
    }

    pub async fn wait_handlers(self: Arc<Self>){
        for handler in self.handlers.write().await.iter(){
            while handler.is_finished(){}
        }
    }
}
