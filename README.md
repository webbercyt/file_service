# Server-Client File Exchanger 
> Server-Client mode implementation to exchange files via websocket

## Server usage example

run: file_server <host> <port> <threads> <file_root_root>
example: 
> file_server 0.0.0.0 80 4 /usr/files/server


shut: 
>x


## Client usage example

run: file_client <host> <port> <file_root_root>

example: > file_client 192.168.0.1 80 /usr/files/client

shut: 
>x


**get: get -{scope:a|s} {file_name}**

get all example: 
>get -a

get single example: 
>get -s server-client.png



**post: post -{scope:a|s} {file_name}**

post all example:
>post -a

post single example:
>post -s client-server.png


## Development setup

1. dependencies

   minimum dependencies are under ./third_party

2. build

   build file_server and file_client seperately


## Messages

JSON messages have defined to indicate requests and results of files transfer

* get file request: from client to server
   * uuid: unique message id, used to trace response
   * method: "get"
   * scope: "single" = single file; "all" = all valid files
   * target: file name, e.g. get_request_target.txt
 ```
Examples
{
	"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
	"method":"get"
	"scope":"all"
}
{
	"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
	"method":"get"
	"scope":"single"
	"target":"get_request_target.txt"
}
```

* post file request: from client to server, or verse
   * uuid: unique message id, used to trace response
   * method: "get"
   * target: file name, e.g. get_request_target.txt
   * context: binary context of file
```
Examples
{
	"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
	"method":"post"
	"target":"post_request_target.txt"
	"context":"FFFFCAE051AE9"
}
```

* response: from server to client
   * uuid: unique message id, used to trace request
   * response: "accepted" or "rejected"
   * reason: interpretation of response (optional) 
```
Examples
{
	"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
	"response":"rejected"
	"reason":"failed to parse message"
}
```
