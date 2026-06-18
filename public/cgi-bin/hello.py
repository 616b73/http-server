#!/usr/bin/env python3
import os

print("Content-Type: text/plain\n")
print("Hello from Python CGI!")
print(f"Request Method: {os.environ.get('REQUEST_METHOD', 'Unknown')}")
