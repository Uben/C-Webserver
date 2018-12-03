## C Web Server
#### Course Project | NJIT CS288

Implement a simple HTTP web server and host your personal web page with the server. The web server should be able to respond to the page requests initiated from a web browser (such as Chrome or Firefox) as a client. The server serves pages stored locally (i.e., on the same computer in which the server is running), including your personal web page.

- Objectives
  - To further improve programing skills in C.
  - To learn how to handle network connections in Linux.
  - To gain deeper understanding on the client-server model and how servers function.
  - To gain deeper understanding on HTTP protocol.
  - To learn HMTL and get familiar with the tags in HTML.

- Overview
  - The web server implemented will be similar to, but much simpler than, normal web servers, such as Apache or IIS.
  - It only responds to GET requests with static content (i.e., files that can be delivered to browsers without having to be generated, modified, or processed).
  - It does not respond to other requests (e.g., POST) but still properly responds correctly to the invalid requests from browsers (e.g., requests for non-existent content) by generating corresponding messages (e.g., "HTTP 404 - Not Found" error message).
  - The program accepts a port number when it is run, and prints out a usage message to instruct the user to do so if the port number is not given in the command line.
  - The command line to run the program is like the following: `./server 13895`
    - `./server` is the executable
    - `13895` is the port number

- Application Step Through
  - accepts connections (TCP) from web browsers;
  - reads in the packets that the browsers sends;
  - detects the end of a request by checking the content in the packets;
  - parses the request to determine the type of HTTP connection (persistent vs. non-persistent) and the content to be served;
  - prepares and sends a response to the web browser. If the request is invalid, the response should contain the corresponding error message. When the response is re