# Webserv

A non-blocking HTTP/1.1 web server written in C++98, inspired by NGINX. The project demonstrates event-driven architecture, socket-level network programming, HTTP request handling, routing, static file serving, uploads and CGI integration.

![C++98](https://img.shields.io/badge/C%2B%2B-98-00599C?style=plastic&logo=c%2B%2B&logoColor=white)
![HTTP](https://img.shields.io/badge/HTTP/1.1-Server-1f6feb?style=plastic)
![CGI](https://img.shields.io/badge/CGI-Supported-4EAA25?style=plastic)
![42 Berlin](https://img.shields.io/badge/Made_for-42_Berlin-black?style=plastic)

## Why this project matters

Webserv is a systems programming project focused on implementing the core behavior of an HTTP server from scratch in C++98. It required handling socket lifecycle, request parsing, routing, static asset delivery, status code correctness, uploads, error handling, and CGI integration, all within a non-blocking event-driven architecture built around `poll()`.

This project demonstrates practical understanding of HTTP server design beyond framework-level web development. It also reflects the resilience requirements of the 42 subject: the server should remain available under load, handle multiple clients, and avoid blocking I/O behavior.

## Project in action

![webserv demo](assets/webserv-demo.gif)

## Requirements

- C++98-compatible compiler
- GNU Make
- Linux or macOS

Optional:
- `php-cgi` for testing PHP CGI routes
- `siege` for resilience / load testing

## Quick start

Build the project:

```bash
make
```

Run the server with a configuration file:

```bash
./webserv config/multiserver.conf
```

Open one of the configured ports in your browser:

```text
http://127.0.0.1:8001/
http://127.0.0.1:8002/
```

### Optional setup for CGI

```bash
sudo apt update
sudo apt install php-cgi
```

## Usage

The executable accepts an optional configuration file path:

```bash
./webserv [configuration_file]
```

Example:

```bash
./webserv config/multiserver.conf
```

## Configuration

The server uses an NGINX-like configuration style with support for:

- server host and port selection
- multiple server blocks
- route-specific method restrictions
- custom roots and index files
- autoindex on/off
- redirects
- upload destinations
- CGI execution by file extension
- default error pages
- client body size limits

Example:

```conf
server {
    listen 8001 8002;
    host 127.0.0.1;

    root www;
    index index.html;
    error_page 404 www/errors/404.html;
    client_max_body_size 2000000;

    location / {
        autoindex on;
        allow_methods GET POST DELETE;
        root www;
        index index.html;
        upload_dir www/uploads/;
        cgi .php /usr/bin/php;
    }

    location /redirect {
        return https://example.com;
    }
}
```

## Supported features

- Non-blocking HTTP server in C++98
- Multiple ports and server blocks
- Static file serving
- Route-based configuration
- Default error pages
- GET, POST, and DELETE methods
- File uploads
- Directory listing / autoindex
- HTTP redirections
- CGI execution for configured file extensions
- Browser-based testing with real HTTP requests

## Architecture

At a high level, Webserv:

1. parses a server configuration file;
2. creates listening sockets for the configured host and port combinations;
3. uses a non-blocking `poll()`-based event loop to manage incoming connections and client I/O;
4. parses HTTP requests incrementally;
5. resolves the matching server and route configuration;
6. serves static files or directory listings where appropriate;
7. handles uploads, deletions, and redirects based on route rules;
8. executes CGI for configured routes and extensions;
9. generates HTTP responses with accurate status codes and error pages.

The project is built around an event-driven design rather than a thread-per-connection model, which keeps the server responsive under concurrent traffic.

## Testing

### Browser testing

Use a browser to verify:

- static page loading
- asset delivery
- bad URLs and error pages
- directory listing
- redirects
- uploads
- multiple configured ports

### Command-line testing

Simple GET request:

```bash
curl http://127.0.0.1:8001/
```

Trigger an error page:

```bash
curl http://127.0.0.1:8001/does-not-exist
```

Upload a file:

```bash
curl -F "file=@Makefile" http://127.0.0.1:8001/uploads
```

Delete a file:

```bash
curl -X DELETE http://127.0.0.1:8001/uploads/Makefile
```

### Load testing

Install `siege` if needed.

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install siege
```

macOS (Homebrew):

```bash id="1kaj3w"
brew install siege
```

Run a basic resilience test:

```bash id="2vyl25"
siege -b -c10 -t10s http://127.0.0.1:8001/
```