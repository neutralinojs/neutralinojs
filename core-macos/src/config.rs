use serde::de::{self, Deserialize, Deserializer};
use serde_derive;
use serde_json;
use std::fs::File;
use std::path::Path;
use std::str::FromStr;
use std::fmt;

#[derive(Debug, Clone, serde_derive::Deserialize, serde_derive::Serialize)]
pub struct Config {
    pub appname: String,
    #[serde(deserialize_with = "de_from_str")]
    pub appport: u64,
    pub mode: Mode,
    pub window: WindowConfig,
}

#[derive(Debug, Clone, serde_derive::Serialize)]
pub enum Mode {
    Browser,
    Window,
}

impl fmt::Display for Mode {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl<'de> Deserialize<'de> for Mode {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
        where D: Deserializer<'de>
    {
        let s = String::deserialize(deserializer)?;
        Ok(match s.as_str() {
            "browser" => Mode::Browser,
            "window" => Mode::Window,
            _ => panic!("Couldn't deserialize mode"),
        })
    }
}

#[derive(Debug, Clone, serde_derive::Deserialize, serde_derive::Serialize)]
pub struct WindowConfig {
    #[serde(deserialize_with = "de_from_str")]
    pub width: u64,
    #[serde(deserialize_with = "de_from_str")]
    pub height: u64,
}

fn de_from_str<'de, D>(deserializer: D) -> Result<u64, D::Error>
    where D: Deserializer<'de>
{
    let s = String::deserialize(deserializer)?;
    u64::from_str(&s).map_err(de::Error::custom)
}

pub fn read_config() -> Config {
    let file_path = Path::new("settings.json");
    let file = File::open(file_path).unwrap();
    let config: Config = serde_json::from_reader(file).unwrap();
    config
}