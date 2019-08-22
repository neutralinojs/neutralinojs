#![windows_subsystem = "windows"]

#[macro_use]
extern crate lazy_static;

use chrono;
use fern;
use log::{debug, info};
use std::env;
use std::fs;
use std::path::PathBuf;
use std::sync::mpsc;
use web_view::*;

mod config;
mod server;

fn setup_logger() -> Result<(), fern::InitError> {
    fern::Dispatch::new()
        .format(|out, message, record| {
            out.finish(format_args!(
                "{}[{}][{}] {}",
                chrono::Local::now().format("[%Y-%m-%d][%H:%M:%S]"),
                record.target(),
                record.level(),
                message
            ))
        })
        .level(log::LevelFilter::Debug)
        .chain(std::io::stdout())
        .chain(fern::log_file("output.log")?)
        .apply()?;
    Ok(())
}

fn main() {
    let res = setup_logger();
    if res.is_err() {
        eprintln!("Failed to init logger");
    }
    info!("{}", "Starting neutralinojs API server");

    let config = config::read_config();
    env::set_var("NL_VERSION", "1.0.8");
    env::set_var("NL_NAME", config.appname.clone());
    //env::set_var("NL_PORT", config.appport.clone());
    env::set_var("NL_MODE", config.mode.to_string());
    //env::set_var("NL_VERSION", "1.0.8");

    let (tx, rx) = mpsc::channel();
    server::run_server(config.clone(), tx);
    let addr = rx.recv().unwrap();
    info!("Started API server");

    let html_path = PathBuf::from("index.html");
    let html_content = fs::read_to_string(html_path).unwrap();
    web_view::builder()
        .title("NeutralinoJS")
        .size(config.window.height as i32, config.window.width as i32)
        .content(Content::Html(html_content))
        .user_data(())
        .debug(true)
        .invoke_handler(|_webview, _arg| Ok(()))
        .run()
        .unwrap();
}