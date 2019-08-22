use actix_web::{
    error, middleware, web, App, HttpRequest, HttpResponse, HttpServer, dev::Server,
};
use actix_files as afs;

use duct::cmd;
use log::{debug, info};
use nfd::Response;
use serde_derive::{Deserialize, Serialize};
use serde_json::json;
use std::env;
use std::fs;
use std::path::PathBuf;
use std::sync::{mpsc::Sender, Arc, Mutex};
use sysinfo::{System, SystemExt};
use std::thread;
use futures::future::Future;
use shell_words::split;

use crate::config::Config;

lazy_static! {
    static ref STATE: Arc<Mutex<State>> = Arc::new(Mutex::new(State::new()));
}

struct State {
    config: Option<Config>,
    system: System,
}

impl State {
    pub fn new() -> State {
        State {
            config: None,
            system: System::new(),
        }
    }
}

pub fn run_server(config: Config, tx: Sender<Server>) {
    let address = format!("127.0.0.1:{}", config.appport);
    STATE.lock().unwrap().config = Some(config);

    info!("Connecting to {}", &address);

    // let (tx, rx) = mpsc::channel();
    thread::spawn(move || {
        let sys = actix_rt::System::new("neutralinojs");

        let addr = HttpServer::new(|| {
            App::new()
                .wrap(middleware::Logger::default())
                .data(web::JsonConfig::default().error_handler(|err, _req| {
                    let response = json!({
                        "success": false,
                        "error": err.to_string(),
                    });
                    error::InternalError::from_response(
                        err,
                        HttpResponse::Ok().json(response)
                    ).into()
                }))
                .service(web::resource("/computer/getRamUsage").route(web::get().to_async(get_ram_usage)))
                .service(web::resource("/filesystem/createDirectory").route(web::post().to_async(create_dir)))
                .service(web::resource("/filesystem/removeDirectory").route(web::post().to_async(remove_dir)))
                .service(web::resource("/filesystem/writeFile").route(web::post().to_async(write_file)))
                .service(web::resource("/filesystem/removeFile").route(web::post().to_async(remove_file)))
                .service(web::resource("/filesystem/readFile").route(web::post().to_async(read_file)))
                .service(web::resource("/filesystem/readDirectory").route(web::post().to_async(read_dir)))
                .service(web::resource("/os/getEnvar").route(web::post().to_async(get_envar)))
                .service(web::resource("/os/dialogOpen").route(web::post().to_async(open_dialog)))
                .service(web::resource("/os/dialogSave").route(web::post().to_async(save_dialog)))
                .service(web::resource("/os/putData").route(web::post().to_async(put_data)))
                .service(web::resource("/os/getData").route(web::post().to_async(get_data)))
                .service(web::resource("/os/runCmd").route(web::post().to_async(run_cmd)))
                .service(web::resource("/settings.json").route(web::get().to_async(get_config)))
                .service(web::resource("/populate_env").route(web::get().to(get_env_js_file)))
                .service(web::resource("/{filename:.*\\.css}").route(web::get().to(get_file_css)))
                .service(web::resource("/{filename:.*\\.js}").route(web::get().to(get_file_js)))
            })
            .bind(address)
            .unwrap()
            .start();

        let _ = tx.send(addr);
        let _ = sys.run();
    });
}

// fixme: for some reason the js doesn't get executed.
fn get_env_js_file(req: HttpRequest) -> HttpResponse {
    info!("Serving env setting js file");
    let unlocked_state = STATE.lock().unwrap();
    match &unlocked_state.config {
        Some(ref config) => {
            let js = format!("var NL_NAME=\"{}\";var NL_VERSION=\"{}\";var NL_MODE=\"{}\";", config.appname, "1.0.8", config.mode.to_string());
            HttpResponse::Ok()
                .content_type("text/javascript")
                .body(js)
        },
        None => {
            let response = json!({
                "success": false,
                "error": "Config not available",
            });
            HttpResponse::Ok().json(response)
        },
    }
}

fn get_file_js(req: HttpRequest) -> HttpResponse {
    let filename_path: String = req.match_info().query("filename").parse().unwrap();
    //let asset_dir_path = PathBuf::from("./assets").join(filename_path);
    info!("Serving {:?}", &filename_path);
    
    match fs::read_to_string(&filename_path) {
        Ok(content) => {
            HttpResponse::Ok()
                .content_type("text/javascript")
                .body(content)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

fn get_file_css(req: HttpRequest) -> HttpResponse {
    let filename_path: String = req.match_info().query("filename").parse().unwrap();
    //let asset_dir_path = PathBuf::from("./assets").join(filename_path);
    info!("Serving {:?}", &filename_path);
    
    match fs::read_to_string(&filename_path) {
        Ok(content) => {
            HttpResponse::Ok()
                .content_type("text/css")
                .body(content)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Serialize)]
struct RamUsageResponse {
    available: u64,
    total: u64,
}

fn get_ram_usage(req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);

    let mut unlocked_state = STATE.lock().unwrap();
    unlocked_state.system.refresh_system();

    let res = RamUsageResponse {
        available: unlocked_state.system.get_free_memory(),
        total: unlocked_state.system.get_total_memory(),
    };
    HttpResponse::Ok().json(res)
}

#[derive(Debug, Deserialize)]
struct CreateDirRequest {
    name: String,
}

fn create_dir(item: web::Json<CreateDirRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let dir_name = &item.name;
    let dir_path = PathBuf::from(&dir_name);
    match fs::create_dir(dir_path) {
        Ok(_) => {
            let response = json!({
                "success": true,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct RemoveDirRequest {
    dir: String,
}

fn remove_dir(item: web::Json<RemoveDirRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let dir_name = &item.dir;
    let dir_path = PathBuf::from(&dir_name);
    match fs::remove_dir(dir_path) {
        Ok(_) => {
            let response = json!({
                "success": true,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct WriteFileRequest {
    filename: String,
    content: String,
}

fn write_file(item: web::Json<WriteFileRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let file_name = &item.filename;
    let file_content = &item.content;
    let file_path = PathBuf::from(&file_name);
    match fs::write(file_path, file_content) {
        Ok(_) => {
            let response = json!({
                "success": true,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct RemoveFileRequest {
    filename: String,
}

fn remove_file(item: web::Json<RemoveFileRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let file_name = &item.filename;
    let file_path = PathBuf::from(&file_name);
    match fs::remove_file(file_path) {
        Ok(_) => {
            let response = json!({
                "success": true,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct ReadFileRequest {
    filename: String,
}

fn read_file(item: web::Json<ReadFileRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let file_name = &item.filename;
    let file_path = PathBuf::from(&file_name);
    match fs::read_to_string(file_path) {
        Ok(content) => {
            let response = json!({
                "success": true,
                "content": content,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct ReadDirRequest {
    path: String,
}

fn read_dir(item: web::Json<ReadDirRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let dir_name = &item.path;
    let dir_path = PathBuf::from(&dir_name);
    let path_entries = match fs::read_dir(dir_path) {
        Ok(paths) => paths,
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            return HttpResponse::Ok().json(response);
        },
    };

    let mut file_list = vec![];
    for entry in path_entries {
        if let Ok(entry) = entry {
            if let Ok(file_type) = entry.file_type() {
                let file_type = if file_type.is_dir() {
                    "dir"
                } else if file_type.is_file() {
                    "file"
                } else {
                    "other"
                };

                file_list.push(json!({
                    "path": entry.path(),
                    "type": file_type,
                }));
            }
        }
    }

    let response = json!({
        "success": true,
        "files": file_list,
    });
    HttpResponse::Ok().json(response)
}

#[derive(Debug, Deserialize)]
struct EnvarRequest {
    name: String,
}

fn get_envar(item: web::Json<EnvarRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let envar_name = &item.name;
    match env::var(envar_name) {
        Ok(value) => {
            let response = json!({
                "success": true,
                "value": value,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

fn get_config(req: HttpRequest) -> HttpResponse {
    let response = serde_json::to_string(&STATE.lock().unwrap().config).unwrap();
    HttpResponse::Ok()
        .content_type("application/json")
        .body(response)
}

#[derive(Debug, Deserialize)]
struct DialogRequest {
    title: String,
}

fn open_dialog(item: web::Json<DialogRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let dialog_title = &item.title;
    let result = nfd::open_dialog(None, None, nfd::DialogType::SingleFile);
    if result.is_err() {
        let response = json!({
            "success": false,
            "error": "Failed to show open dialog",
        });
        return HttpResponse::Ok().json(response);
    }

    match result.unwrap() {
        Response::Okay(path) => {
            let response = json!({
                "success": true,
                "file": path,
            });
            HttpResponse::Ok().json(response)
        },
        _ => {
            let response = json!({
                "success": false,
                "error": "Failed to open file",
            });
            HttpResponse::Ok().json(response)
        }
    }
}

fn save_dialog(item: web::Json<DialogRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let dialog_title = &item.title;
    let result = nfd::open_dialog(None, None, nfd::DialogType::SaveFile);
    if result.is_err() {
        let response = json!({
            "success": false,
            "error": "Failed to show save dialog",
        });
        return HttpResponse::Ok().json(response);
    }

    match result.unwrap() {
        Response::Okay(path) => {
            let response = json!({
                "success": true,
                "file": path,
            });
            HttpResponse::Ok().json(response)
        },
        _ => {
            let response = json!({
                "success": false,
                "error": "Failed to save file",
            });
            HttpResponse::Ok().json(response)
        }
    }
}

#[derive(Debug, Deserialize)]
struct PutDataRequest {
    bucket: String,
    content: String,
}

fn put_data(item: web::Json<PutDataRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let content = &item.content;
    let bucket = &item.bucket;
    let file_name = format!("{}.json", bucket);
    let file_path = PathBuf::from(format!("storage/{}", file_name));

    match fs::write(file_path, content) {
        Ok(_) => {
            let response = json!({
                "success": true,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct GetDataRequest {
    bucket: String,
}

fn get_data(item: web::Json<GetDataRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let bucket = &item.bucket;
    let file_name = format!("{}.json", bucket);
    let file_path = PathBuf::from(format!("storage/{}", file_name));

    match fs::read_to_string(file_path) {
        Ok(content) => {
            let response = json!({
                "success": true,
                "content": content,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}

#[derive(Debug, Deserialize)]
struct RunCmdRequest {
    command: String,
}

fn run_cmd(item: web::Json<RunCmdRequest>, req: HttpRequest) -> HttpResponse {
    debug!("Request: {:?}", req);
    debug!("Model: {:?}", item);

    let command = &item.command;
    let mut splits = split(command).unwrap();
    debug!("{:?}", &splits);

    let cmd_name = splits[0].clone();
    splits.remove(0);

    match cmd(cmd_name, splits).read() {
        Ok(stdout) => {
            let response = json!({
                "success": true,
                "stdout": stdout,
            });
            HttpResponse::Ok().json(response)
        },
        Err(e) => {
            let response = json!({
                "success": false,
                "error": e.to_string(),
            });
            HttpResponse::Ok().json(response)
        },
    }
}
