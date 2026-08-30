# Unix Networking in C

This repository contains small Unix networking programs that I wrote while studying [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/). The core TCP, UDP, `select()`, and `poll()` examples are based on the guide, with small changes and experiments added as I worked through them. One example is the nickname support in the `select()` chat server.

I have kept the repository public as a practical reference for anyone beginning with socket programming in C.

## Programs

| Directory | Program | Description |
| --- | --- | --- |
| `TCP` | `server.c` | Listens on port `3490`, accepts TCP connections, forks a child process, and sends a greeting |
| `TCP` | `client.c` | Resolves a hostname, connects to the server, and prints the received message |
| `UDP` | `datagramserver.c` | Binds an IPv6 datagram socket on port `4950` and receives one packet |
| `UDP` | `datagramclient.c` | Resolves a host and sends a message using `sendto()` |
| `Multichat Server` | `multichatserver.c` | Uses `poll()` to relay messages between multiple connected clients |
| `Multichat Server` | `selectmserver.c` | Uses `select()` for multi-client chat and supports `/nick <name>` |
| `misc` | `findip.c` | Prints the IPv4 and IPv6 addresses returned by `getaddrinfo()` |
| `misc` | `pollexample.c` | Demonstrates waiting for standard input with `poll()` and a timeout |

## What this repository covers

- resolving hostnames with `getaddrinfo()`;
- creating TCP and UDP sockets;
- binding, listening, accepting, connecting, sending, and receiving;
- working with IPv4 and IPv6 addresses;
- handling concurrent TCP connections with `fork()`;
- monitoring multiple file descriptors with `select()` and `poll()`;
- broadcasting messages between connected clients.

## Requirements

The programs were written and tested on Linux and WSL. You will need:

- GCC, Clang, or another C compiler;
- standard POSIX networking headers;
- Telnet / Netcat if you want to make the chat servers talk!;

On Ubuntu or Debian:

```bash
sudo apt install build-essential netcat-openbsd
```

## Build

Run the following commands from the repository root:

```bash
mkdir -p build

cc -Wall -Wextra TCP/server.c -o build/tcp-server
cc -Wall -Wextra TCP/client.c -o build/tcp-client

cc -Wall -Wextra UDP/datagramserver.c -o build/udp-server
cc -Wall -Wextra UDP/datagramclient.c -o build/udp-client

cc -Wall -Wextra "Multichat Server/multichatserver.c" -o build/chat-poll
cc -Wall -Wextra "Multichat Server/selectmserver.c" -o build/chat-select

cc -Wall -Wextra misc/findip.c -o build/findip
cc -Wall -Wextra misc/pollexample.c -o build/poll-example
```

## Running the examples

### TCP client and server

Start the server:

```bash
./build/tcp-server
```

Connect from another terminal:

```bash
./build/tcp-client localhost
```

The client should receive `Hello, world!` and then exit.

### UDP client and server

Start the UDP listener:

```bash
./build/udp-server
```

Send it a message from another terminal:

```bash
./build/udp-client ::1 "hello over UDP"
```

### Multi-client chat

Run either the `poll()` or `select()` implementation. Both use port `9034`, so they cannot run at the same time.

```bash
./build/chat-poll
# or
./build/chat-select
```

Connect from two or more terminals:

```bash
nc localhost 9034
```

Messages from one client are forwarded to the others. The `select()` version also accepts:

```text
/nick cheesecake
```
