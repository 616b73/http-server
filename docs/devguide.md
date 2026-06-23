# HTTP Server Implementation Walkthrough

We have successfully transformed a single-connection socket program into a fully-functional, multi-process HTTP server. Below is a summary of the architectural changes and what was accomplished during this project.

## Changes Made

### 1. Project Structure and Compilation
- **Makefile**: We added a standard `Makefile` that compiles `src/server.c` into an executable called `http-server`. This ensures builds are reproducible and scalable as we add more files.

### 2. HTTP Request Parsing
- Instead of just printing raw bytes, the server now parses the HTTP **Request Line** using `sscanf()`. It successfully extracts the `Method` (e.g., GET), the `URI` (e.g., `/index.html`), and the `HTTP Version`.
- It now correctly identifies the end of the HTTP headers (`\r\n\r\n`) so that it doesn't wait indefinitely for the client to close the connection.

### 3. Serving Static Files (The `public` Directory)
- We implemented a routing mechanism:
  - We created a `public` directory serving as the root folder for web assets.
  - When a user requests `/`, the server automatically routes them to `public/index.html`.
  - The server utilizes `open()`, reads the file size with `fstat()`, and sends back dynamic `Content-Length` headers along with the binary content of the file.
- If a client requests a file that does not exist, the server sends a correct `404 Not Found` response.

### 4. Concurrency Model (`fork()`)
- To handle multiple users concurrently, we integrated a multi-process architecture using `fork()`.
- **How it works**: 
  - The parent process sits in an infinite loop calling `accept()`. 
  - When a client connects, the parent `fork()`s a child process.
  - The child process handles reading, parsing, and serving the requested file, then exits.
  - The parent immediately loops back to `accept()`, ready for the next client without waiting.
- **Zombie Process Cleanup**: We added a signal handler for `SIGCHLD` using `sigaction()` and `waitpid()` to ensure that when child processes finish, their resources are properly reaped by the OS.

### 5. POST Request Handling
- Building upon the new `HttpHeader` parsing logic, the server now intelligently parses arbitrary HTTP headers, particularly looking for `Content-Length`.
- When a `POST` request is made to the `/echo` endpoint, the server dynamically allocates memory, reads the entire body from the socket (spanning multiple `read()` calls if necessary), and sends back an exact copy of the payload to the client.

### 6. Architectural Refactoring
- To prepare the server for larger-scale feature additions, we separated the monolithic `server.c` into distinct, maintainable modules:
  - **`http_parser`**: Extracts raw socket data and populates a structured `HttpRequest` object.
  - **`router`**: Inspects the method and URI to determine the correct response path.
  - **`response`**: Handles formatting valid HTTP responses and transmitting file buffers.
  - **`server`**: Now purely dedicated to network socket setup and process management.

### 7. Thread Pool Concurrency Upgrade
- We replaced the heavy, process-based `fork()` concurrency model with a lightning-fast **Thread Pool** architecture using POSIX threads (`pthreads`).
- **How it works:**
  - Upon startup, the server spawns a fixed pool of worker threads (e.g., 8).
  - The main thread rapidly loops over `accept()` and immediately pushes the new client connection file descriptors into a custom, thread-safe **queue**.
  - Worker threads sleep efficiently using condition variables (`pthread_cond_t`) until a connection is available. They then wake up, pop the connection from the queue, parse/route/respond, and then close the connection before waiting for the next one.
- This approach drastically reduces the overhead of handling high volumes of concurrent requests while seamlessly integrating with our blocking parsing logic.

### 8. MIME Type Mapping
- The server now dynamically determines the `Content-Type` header based on the file extension of the requested file.
- It correctly serves `.html`, `.css`, `.js`, `.json`, `.jpg`, `.png`, and defaults to `application/octet-stream` for unknown types.

### 9. Common Gateway Interface (CGI) Support
- The server is no longer limited to static files! It can now execute dynamic scripts (like Python or Bash) and stream their output directly to the client.
- **How it works:**
  - Any request with a URI starting with `/cgi-bin/` is mapped to the `public/cgi-bin/` directory and routed to a dedicated `handle_cgi()` function.
  - The server creates two **pipes** for inter-process communication, uses `fork()` to spawn a child process, sets basic CGI environment variables (like `REQUEST_METHOD` and `CONTENT_LENGTH`), and executes the script using `execl()`.
  - Using `dup2()`, the child's `stdin` and `stdout` are redirected to the pipes. The worker thread writes the POST body (if any) to the script, then reads the script's output and streams it back to the client socket.

### 10. Security (Path Traversal Protection)
- Added security checks in the router to explicitly prevent clients from requesting files outside the designated `public/` folder.
- If a client requests a URI containing `..` (e.g., `/../../etc/passwd`), the server immediately rejects the request and returns a `403 Forbidden` response.

### 11. Keep-Alive Connections (Persistent Connections)
- Upgraded the server to support HTTP `Keep-Alive`, allowing clients to reuse the same TCP socket for multiple sequential requests.
- **How it works:**
  - The `http_parser` extracts the `Connection` header. If it is set to `keep-alive`, a flag is passed down the pipeline.
  - In `server.c`, the worker thread enters a `do-while` loop for that client socket instead of closing it immediately.
  - A 5-second socket read timeout (`SO_RCVTIMEO`) is enforced to ensure the worker thread does not hang indefinitely if the client idles or drops the connection.
  - Responses dynamically inject `Connection: keep-alive` headers so the client knows the connection remains open.
