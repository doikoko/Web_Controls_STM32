use std::{env::current_dir, io::Read, path::PathBuf, sync::Arc, time::Duration};
use tokio_serial::{DataBits, FlowControl, Parity, SerialPortBuilderExt, SerialStream, StopBits};
use tokio::{io::{split, ReadHalf, WriteHalf}, sync::RwLock, task::JoinHandle, time::sleep};
use warp::{path, reply::html, serve, Filter};
use serde::Serialize;

type ArcRw<T> = Arc<RwLock<T>>;
struct USART{
    tx: ArcRw<WriteHalf<SerialStream>>,
    rx: ArcRw<ReadHalf<SerialStream>>,
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>
}
impl USART{
    async fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self {      
        let mut port = None;
        while !mcu_data.read().await.is_active {
            port = match tokio_serial::new(&mcu_data.read().await.name, 115200)
                    .timeout(Duration::from_millis(100))
                    .data_bits(DataBits::Eight)
                    .flow_control(FlowControl::None)
                    .parity(Parity::Even)
                    .stop_bits(StopBits::One)
                    .open_native_async(){
                        Ok(stream) => {
                            mcu_data.write().await.is_active = true;
                            Some(stream)
                        },
                        Err(_) => { 
                            sleep(Duration::from_millis(100)).await;
                            break;
                            //continue;
                        }
                    }
                }
        let (rx, tx) = split(port.unwrap());
        
        println!("successfully connected to serial port {}", mcu_data.read().await.name);
        
        Self {
            tx: Arc::new(RwLock::new(tx)),
            rx: Arc::new(RwLock::new(rx)),
            mcu_data: mcu_data,
            user_code: user_code
        }
    }

    async fn send_code(&self){}
}

struct ServerFiles{
    manifest_dir: PathBuf,
    web_dir: PathBuf,
    index_html: PathBuf
}
async fn foo(){}
struct Server{
    mcu_data: ArcRw<MCUData>,
    user_code: ArcRw<Code>,
    files: ArcRw<ServerFiles>,
    html_data: ArcRw<String>,
    handlers: ArcRw<Vec<JoinHandle<()>>>
}
impl Server{
    async fn new(mcu_data: ArcRw<MCUData>, user_code: ArcRw<Code>) -> Self {
        let manifest_dir = PathBuf::from(current_dir().unwrap());

        let files = ServerFiles {
            manifest_dir:   manifest_dir.clone(),
            web_dir:        manifest_dir.clone().join("web"),
            index_html:     manifest_dir.clone().join("index.html"),
        };

        let html_data = tokio::fs::read_to_string(&files.index_html).await.unwrap();

        Self {
            mcu_data:   mcu_data, 
            user_code:  user_code,
            files:      Arc::new(RwLock::new(files)),
            html_data:  Arc::new(RwLock::new(html_data)),
            handlers:   Arc::new(RwLock::new(Vec::new()))
        }       
    }

    async fn configure(self: Arc<Self>, usart: Arc<USART>){
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
                    let usart = usart.clone();
                    async move {
                        (*server).user_code.write().await.0 = code;
                        server.handlers.write().await.push(tokio::spawn(async move{
                            usart.send_code().await
                        }));
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
}

#[derive(Serialize)]
struct MCUData{
    name: String,
    is_active: bool,
    temperature: u8,
}
impl MCUData{
    fn new() -> Self {
        println!("input serial port path\nExample: /dev/ttyUSB0");

        let mut name = String::new();
        std::io::stdin().read_to_string(&mut name).unwrap();
        
        Self {
            name: name.trim().to_string(),
            is_active: false,
            temperature: 0
        }
    }
}

struct Code(String);
impl Code{
    fn new() -> Self{
        Self (String::new())
    }
}

#[tokio::main]
async fn main() -> std::io::Result<()>{
    let mcu_data = Arc::new(RwLock::new(MCUData::new()));
    let user_code = Arc::new(RwLock::new(Code::new()));

    let (usart, server) = tokio::join!(
        async {
            Arc::new(USART::new(mcu_data.clone(), user_code.clone()).await)
        },
        async {
            Arc::new(Server::new(mcu_data.clone(), user_code.clone()).await)
        }
    );


    let mut res = Some(tokio::spawn(server.clone().configure(usart)));
    tokio::select!(
        _ = res.as_mut().unwrap() => {},
        _ = tokio::signal::ctrl_c() => {
            println!("shutting down...");
            for handler in (*server.handlers.write().await).drain(..) {
                handler.await.unwrap();
            }
            res.as_mut().unwrap().abort();
        }
    );

    Ok(())
}
