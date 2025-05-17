# Simple C++ HTTP Server

A minimal, POSIX-socket-based HTTP/1.1 server written in C++.  
Supports basic routing, static file serving from a specified directory, and file upload via `POST`.  

---

## Features

- **GET /**  
  Returns a `200 OK` with an empty body.

- **GET /echo/{text}**  
  Returns `200 OK` with a plain-text body of `{text}`.

- **GET /user-agent**  
  Returns `200 OK` with your `User-Agent` header value.

- **GET /files/{filename}**  
  Serves the file named `{filename}` from a user-specified directory (via `--directory`).  
  - If the file exists:  
    ```http
    HTTP/1.1 200 OK
    Content-Type: application/octet-stream
    Content-Length: <size>

    <raw file bytes>
    ```
  - If not found, returns `404 Not Found`.

- **POST /files/{filename}**  
  Creates or overwrites `{filename}` in the same directory, writing the raw request body.  
  - On success:  
    ```http
    HTTP/1.1 201 Created
    Content-Length: 0
    ```
  - On failure, returns `404 Not Found` or `500 Internal Server Error` as appropriate.

---

## Requirements

- Linux, macOS, or any POSIX-compatible OS
- A C++17-compatible compiler (e.g. `g++`, `clang++`)

---

## Building

```bash
git clone https://github.com/<your-username>/simple-cpp-http-server.git
cd simple-cpp-http-server

# Build:
g++ -std=c++17 -pthread -o http-server src/server.cpp
