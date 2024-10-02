use get_if_addrs::{get_if_addrs, IfAddr};
use std::net::{IpAddr, Ipv4Addr, SocketAddrV4, UdpSocket};
use std::str;
use std::sync::{Arc, Mutex};

const BROADCAST_PORT: u16 = 6666;
const MULTICAST_PORT: u16 = 7777;
const MULTICAST_ADDR: &str = "224.1.1.1:7777";

fn main() -> std::io::Result<()> {
    // Получение IP-адреса, маски и широковещательного адреса
    let mut local_addr = Ipv4Addr::new(0, 0, 0, 0);
    let mut broadcast_addr = Ipv4Addr::new(255, 255, 255, 255);

    if let Ok(interfaces) = get_if_addrs() {
        for iface in interfaces {
            if let IfAddr::V4(v4_addr) = iface.addr {
                if !v4_addr.ip.is_loopback() {
                    local_addr = v4_addr.ip;
                    broadcast_addr = v4_addr
                        .broadcast
                        .unwrap_or(Ipv4Addr::new(255, 255, 255, 255));
                    println!(
                        "Используем интерфейс: {} с широковещательным адресом: {}",
                        local_addr, broadcast_addr
                    );
                    break;
                }
            }
        }
    }

    if local_addr.is_unspecified() {
        eprintln!("Не удалось определить локальный IP-адрес");
        return Ok(());
    }

    // Сокеты для приема
    let receive_socket_broadcast = UdpSocket::bind((Ipv4Addr::UNSPECIFIED, BROADCAST_PORT))?;
    receive_socket_broadcast.set_broadcast(true)?;

    let receive_socket_multicast =
        Arc::new(UdpSocket::bind((Ipv4Addr::UNSPECIFIED, MULTICAST_PORT))?);
    let multicast_addr = MULTICAST_ADDR.parse::<SocketAddrV4>().unwrap();
    receive_socket_multicast.join_multicast_v4(multicast_addr.ip(), &local_addr)?;

    let send_socket = UdpSocket::bind((local_addr, 0))?;
    send_socket.set_broadcast(true)?;

    // Общий черный список (используем Arc и Mutex для доступа из нескольких потоков)
    let blacklist = Arc::new(Mutex::new(Vec::new()));

    let blacklist_multicast = Arc::clone(&blacklist);
    let blacklist_broadcast = Arc::clone(&blacklist);
    let receive_socket_multicast_clone = Arc::clone(&receive_socket_multicast); // Clone the Arc

    // Поток для получения broadcast сообщений
    std::thread::spawn(move || {
        let mut buf = [0; 1024];
        loop {
            match receive_socket_broadcast.recv_from(&mut buf) {
                Ok((size, addr)) => {
                    let blacklist = blacklist_broadcast.lock().unwrap();
                    if blacklist.contains(&addr.ip()) {
                        println!("Сообщение от {} проигнорировано (в черном списке)", addr);
                        continue;
                    }
                    let msg = str::from_utf8(&buf[..size]).unwrap();
                    println!("Получено broadcast сообщение от {}: {}", addr, msg);
                }
                Err(e) => {
                    eprintln!("Ошибка при получении broadcast данных: {}", e);
                }
            }
        }
    });

    // Поток для получения multicast сообщений
    std::thread::spawn(move || {
        let mut buf = [0; 1024];
        loop {
            match receive_socket_multicast_clone.recv_from(&mut buf) {
                Ok((size, addr)) => {
                    let blacklist = blacklist_multicast.lock().unwrap();
                    if blacklist.contains(&addr.ip()) {
                        println!("Сообщение от {} проигнорировано (в черном списке)", addr);
                        continue;
                    }
                    let msg = str::from_utf8(&buf[..size]).unwrap();
                    println!("Получено multicast сообщение от {}: {}", addr, msg);
                }
                Err(e) => {
                    eprintln!("Ошибка при получении multicast данных: {}", e);
                }
            }
        }
    });

    println!("Введите сообщение для отправки ('leave' для выхода, 'block [IP]' для игнорирования, 'exit' для завершения):");

    loop {
        let mut input = String::new();
        std::io::stdin().read_line(&mut input)?;
        let msg = input.trim();

        if msg == "exit" {
            break;
        }

        send_socket.send_to(msg.as_bytes(), &(broadcast_addr.to_string() + ":6666"))?;

        if msg == "leave" {
            receive_socket_multicast.leave_multicast_v4(multicast_addr.ip(), &local_addr)?;
        } else {
            send_socket.set_multicast_ttl_v4(10)?;
            send_socket.send_to(msg.as_bytes(), MULTICAST_ADDR)?;
        }

        if msg.starts_with("block") {
            let parts: Vec<&str> = msg.split_whitespace().collect();
            if parts.len() == 2 {
                if let Ok(block_ip) = parts[1].parse::<Ipv4Addr>() {
                    let mut blacklist = blacklist.lock().unwrap();
                    blacklist.push(IpAddr::V4(block_ip));
                    println!("IP-адрес {} добавлен в черный список", block_ip);
                } else {
                    println!("Некорректный IP-адрес");
                }
            } else {
                println!("Неверный формат команды. Используйте 'block [IP]'");
            }
        }
    }

    Ok(())
}
