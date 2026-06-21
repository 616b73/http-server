# Custom C HTTP Server - Overview

Welcome to the **Custom C HTTP Server**! This server was built from scratch in C to demonstrate the foundational concepts of networking, concurrent architectures, and the HTTP protocol.

## Capabilities

This server is highly capable and supports the following features:

- **Static File Serving**: Serves HTML, CSS, JavaScript, JSON, and images directly from the `public/` directory.
- **Dynamic MIME Type Mapping**: Automatically detects file extensions and serves the correct `Content-Type` header to ensure browsers render assets accurately.
- **Concurrent Thread Pool**: Built with a highly scalable POSIX threads (`pthreads`) architecture. A pool of worker threads efficiently processes thousands of concurrent requests without the heavy overhead of `fork()`.
- **CGI (Common Gateway Interface)**: Capable of executing dynamic scripts on the fly! Any request to `/cgi-bin/` will spawn a secure process, execute the script (e.g., Python, Bash), and pipe the standard output back to the client.
- **POST Request Support**: Can parse and handle arbitrary HTTP headers and fully read `Content-Length` payloads for POST requests (try the `/echo` endpoint!).
- **Security**: Includes built-in path traversal protection to prevent malicious actors from accessing sensitive files outside of the designated `public/` root folder.

## How to Use It

Using the server is incredibly simple:

1. **Start the Server**: Simply run the `http-server` executable from your terminal. It will bind to port `8080`.
2. **Accessing the Site**: Open your web browser and navigate to `http://localhost:8080/`. You will be served the `public/index.html` file.
3. **Running Scripts**: Place executable scripts in `public/cgi-bin/` (e.g., `hello.py`). You can execute them by navigating to `http://localhost:8080/cgi-bin/hello.py`.

Enjoy a blazing-fast, lightweight HTTP server that you have complete control over!
