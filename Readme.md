## HTTP Proxy
A C++11 http proxy

## Compilation
This project uses CMake. To build, do the following:

1. cd build
2. cmake ..
3. make

## Features
1. This HTTP proxy caches pages and DNS lookups as they are requested by users.
2. A blacklist blacklists requests as they arrive.
3. Multithreaded

## Usage
Start:
`http_proxy [port] [page cache timeout (optional)]`



