# Custom C HTTP Server

A lightweight, concurrent, and modular HTTP server written entirely in C. 
This project was developed from a simple single-connection socket program into a fully-fledged, multi-threaded web server capable of handling static assets, POST requests, and dynamic CGI execution.

## Features
- Thread Pool concurrency model (`pthreads`)
- Static file serving with dynamic MIME types
- Dynamic script execution via CGI (Common Gateway Interface)
- Path traversal security protection
- Fully modular architecture separating network, routing, parsing, and response logic

## Getting Started

### Prerequisites
- GCC compiler
- POSIX compliant OS (Linux/macOS)
- `make` utility

### Installation & Execution

1. Clone the repository and navigate into the project directory:
   ```bash
   git clone <repository_url>
   cd http-server
   ```

2. Compile the server using the provided Makefile:
   ```bash
   make
   ```

3. Run the server:
   ```bash
   ./http-server
   ```
   *The server will start listening for connections on `http://localhost:8080`.*

### Usage Examples

- **View Homepage**: `curl -i http://localhost:8080/`
- **Execute CGI Script**: `curl -i http://localhost:8080/cgi-bin/hello.py`
- **Test POST Echo**: `curl -i -X POST -d "Hello Server!" http://localhost:8080/echo`

## Documentation
- Read the [Overview](docs/overview.md) for a summary of capabilities.
- Read the [Developer Guide](docs/devguide.md) for a complete history of the project's architectural evolution and implementation details.
