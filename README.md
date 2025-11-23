# Tiny TCP Chat (C)

Minimal TCP chat application written in C using BSD sockets and `select()`.  
The project consists of a simple server and client:

- The **server** accepts up to 2 clients and relays messages between them.
- The **client** connects to the server and lets you chat via the terminal.

This is meant as a small learning project to practice:
- TCP client–server programming
- Using `select()` with multiple file descriptors
- Basic network debugging with Wireshark

---

## Features

- TCP server listening on a configurable port (default 5555)
- Supports up to 2 concurrent clients
- Uses `select()` to:
  - Handle new connections
  - Read from multiple connected clients
- Client uses `select()` to read from:
  - `stdin` (keyboard input)
  - the server socket (incoming chat messages)
- Graceful handling of disconnects
- Simple line-based text chat

---

## Requirements

- GCC or any C compiler supporting POSIX sockets
- POSIX-compatible system (Linux, macOS, WSL, etc.)
- Optional: Wireshark if you want to inspect the TCP traffic

---

## Files

- `server.c` – TCP chat server
- `client.c` – TCP chat client

---

## Build

From the project directory:

```bash
gcc server.c -o server
gcc client.c -o client
```

If you want to enable extra warnings (recommended):

```bash
gcc -Wall -Wextra -pedantic server.c -o server
gcc -Wall -Wextra -pedantic client.c -o client
```

---

## Run

### 1. Start the server

In one terminal:

```bash
./server
```

By default the server listens on port `5555` on all interfaces (`0.0.0.0`).

### 2. Start one or two clients

In another terminal:

```bash
./client
```

You should see:

```text
Connected to 127.0.0.1:5555
Type messages and press Enter. Ctrl+C to quit.
```

Optionally start a second client in a third terminal:

```bash
./client
```

Now:

- Anything typed in client 1 is relayed to client 2.
- Anything typed in client 2 is relayed to client 1.
- The server prints basic connection and message logs.

---

## How it works (high-level)

### Server

- Creates a listening socket with:
  - `socket(AF_INET, SOCK_STREAM, 0)`
  - `bind()` to `INADDR_ANY:5555`
  - `listen()`
- Uses `select()` to:
  - Watch the listening socket (new connections)
  - Watch all active client sockets (incoming messages)
- On new connections:
  - Accepts with `accept()`
  - Stores the client file descriptor in a small array (max 2)
- On incoming data from a client:
  - Reads with `recv()`
  - If `recv()` returns `0` or `< 0`: closes and removes the client
  - Otherwise: relays the message to the other connected client(s) with `send()`

### Client

- Connects to `127.0.0.1:5555` using `socket()` + `connect()`
- Uses `select()` to monitor:
  - `STDIN_FILENO` (keyboard)
  - The server socket
- When there is input on `stdin`:
  - Reads a line with `fgets()`
  - Sends it to the server with `send()`
- When there is data from the server:
  - Reads with `recv()`
  - Prints to the terminal

---

## Inspecting traffic with Wireshark (optional)

If you want to see what the chat looks like “on the wire”:

1. Start the server and at least one client so traffic is flowing.
2. Open Wireshark and start capturing on the loopback interface:
   - On Linux this is usually `lo`
   - On macOS this is usually `lo0`
3. Apply a filter like:

   ```text
   tcp.port == 5555
   ```

4. Type messages in the client and watch the TCP stream in Wireshark.

You should be able to follow the TCP stream and see your chat messages inside the packets.

---

## Notes

- This is intentionally small and minimal for learning and demo purposes.
- There is no authentication, encryption, or advanced error handling.
- If you want to extend it, possible ideas:
  - Support more clients
  - Add simple usernames
  - Add command-line arguments for host/port
  - Use non-blocking sockets instead of `select()`
