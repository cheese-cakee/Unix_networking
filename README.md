# Unix Networking in C

A hands-on collection of small C programs for learning Unix network programming from the socket API upward.

This repository follows my progression from resolving a hostname and sending a single packet to serving multiple connected clients with `select()` and `poll()`. The programs are deliberately small enough to read in one sitting, experiment with, and debug using ordinary Linux tools.

## What is included

| Area | Program | What it demonstrates |
| --- | --- | --- |
| TCP | `TCP/server.c` | A concurrent IPv4 server that accepts connections, forks a child, and sends a greeting |
| TCP | `TCP/client.c` | Hostname resolution, connection attempts across returned addresses, and receiving data |
| UDP | `UDP/datagramserver.c` | Binding an IPv6 datagram socket and receiving a packet with `recvfrom()` |
| UDP | `UDP/datagramclient.c` | Resolving a host and sending a connectionless packet with `sendto()` |
| Multi-client I/O | `Multichat Server/multichatserver.c` | A `poll()`-based chat relay with a dynamically growing descriptor array |
| Multi-client I/O | `Multichat Server/selectmserver.c` | A `select()`-based chat server with broadcasting and `/nick` support |
| Utilities | `misc/findip.c` | Printing the IPv4 and IPv6 addresses returned by `getaddrinfo()` |
| Utilities | `misc/pollexample.c` | Waiting for standard input with `poll()` and a timeout |

## Concepts covered

- the `socket()` → `bind()` → `listen()` → `accept()` lifecycle;
- client connections with `connect()`;
- TCP streams versus UDP datagrams;
- IPv4 and IPv6 address handling;
- hostname resolution with `getaddrinfo()`;
- converting binary addresses with `inet_ntop()`;
- process-per-connection concurrency with `fork()`;
- I/O multiplexing with `select()` and `poll()`;
- tracking, accepting, and broadcasting between multiple clients.

## Requirements

- Linux, WSL, or another POSIX-like environment;
- a C compiler such as GCC or Clang;
- optionally, Netcat (`nc`) for connecting to the chat servers.

On Ubuntu or Debian:

```bash
sudo apt install build-essential netcat-openbsd
```

## Build

From the repository root:

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

## Try the examples

### TCP greeting server

Start the server, which listens on port `3490`:

```bash
./build/tcp-server
```

In another terminal, connect the client:

```bash
./build/tcp-client localhost
```

The server forks a child for the connection and the client receives `Hello, world!`.

### UDP datagram

Start the IPv6 UDP listener on port `4950`:

```bash
./build/udp-server
```

Send it a message from another terminal:

```bash
./build/udp-client ::1 "hello over UDP"
```

Unlike TCP, no persistent connection is established: the client sends one datagram and exits.

### Multi-client chat

Choose either implementation; both listen on port `9034`, so run only one at a time:

```bash
./build/chat-poll
# or
./build/chat-select
```

Connect from two or more terminals:

```bash
nc localhost 9034
```

Messages are broadcast to the other connected clients. The `select()` implementation also supports changing the generated nickname:

```text
/nick cheesecake
```

### Resolve a hostname

```bash
./build/findip example.com
```

### Minimal `poll()` demonstration

```bash
./build/poll-example
```

Press Enter to trigger the readable event, or wait 2.5 seconds to observe the timeout.

## Suggested reading order

1. `misc/findip.c`
2. `TCP/server.c` and `TCP/client.c`
3. `UDP/datagramserver.c` and `UDP/datagramclient.c`
4. `misc/pollexample.c`
5. `Multichat Server/multichatserver.c`
6. `Multichat Server/selectmserver.c`

That order moves from address resolution and basic socket communication to managing many connections in a single event loop.

## Scope

These are educational programs, not production-ready servers. They intentionally leave room for later exercises such as message framing, non-blocking sockets, partial-write handling, authentication, resource limits, graceful shutdown, and stronger input validation.

## Acknowledgements

The early socket examples and several conventions in this repository follow the excellent [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/), a practical introduction to sockets in C.
