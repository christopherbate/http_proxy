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
4. Link prefetching
5. Handles different types of HTTP encodings (chunked, normal, etc) efficiently for pipelining.

## Usage
Start:
`http_proxy [port] [page cache timeout (optional)]`

## Code Organization
The code is roughly organized into the following classes/modules:
1. TCPSocket: A wrapper around the TCP socket setup/teardown
2. FileCache: Manages the cache data structures for caching files to disk. Also handles launching threads to pre-fetch links.
3. HTTPRequest: Handles parsing and forming correct HTTPRequests
4. HTTPResponse: Handles parsing and forming correct HTTPResponses. Also handles identifying HTTPChunks in chunked encoding and determining when all data has been received.
5. Session: Session manages an individual session with a client request. Handles processing client request, forwarding to server, and serving the response back to client.
6. UrlCache: Handles looking making all DNS requests and caching the results. Also handles checking these results against the "blacklist". 

# Overall functionality
Sequnce of events:

1. Incoming GET request
2. Launch Session in new thread, pass socket to session
3. Socket receives Data, and Session creates a new HTTPRequest object, passing in the data at intiialization. 
4. Request object pulls out the required header information, and maniuplates the header as necessary (ensuring new request line and host fields are well-formed). Throws exceptions if anything is off.
5. Check the target host against the UrlCache. This automatically checks against blacklist and checks for host availability. Exceptions are thrown by UrlCache if any issues.
7. Check the requested URL against the cache. If it exista and is still good, we have a cache "hit", and we simply open that file and copy to client socket.
6. If cache "miss", open up a new socket and initiate connection using address from UrlCache. Send the new request object.
7. Monitor the incoming data stream from remote host, and copy to client socket. Also copy data to cache. We can handle both chunked and normal encodings. 
8. Once all data has been copied, tell the FileCache to initiate link-prefetch-processign for that file.
9. Return to step 3 until client is finished. 

## Design decisions
I organized my code design around the objects I thought were most important: sessions, responses, requests, caches, and sockets. I wrapped the blacklist functionality into the UrlCache, which made things more efficient so another object did not have to interact with the incoming DNS functionality. I wrapped the pre-fetch functionality into the file cache since they really need to work hand-in-hand. I used regular expressions to efficiently (in terms of coding time and processing time) identify well formed links in the pages which are already cached. The cost complicated parts of the code were monitoring the HTTP download process so that I could quickly return to polling the user. I implemented both chunked-encoding and normal "single-shot" downloading mechanisms. 



