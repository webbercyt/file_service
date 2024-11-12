# Server-Client File Exchanger 
> Server-Client mode implementation to exchange files via websocket

![](infrastructure.png)

See

## Server usage example

**[run]** file_server <host> <port> <threads> <file_root_root>

example: 
> file_server 0.0.0.0 80 4 /usr/files/server


**[shut]**
>x


## Client usage example

**[run]** file_client <host> <port> <file_root_root>

example: 
> file_client 192.168.0.1 80 /usr/files/client


**shut**: 
>x


**[get]** get -{scope:a|s} {file_name}

example: 
>get -a

or
>get -s server-client.png



**[post]** post -{scope:a|s} {file_name}

example:
>post -a

or
>post -s client-server.png


## Development setup

1. Dependencies

   minimum dependencies are stored under ./third_party, no more dependency setup is required.

2. Build

   cmake build file_server and file_client seperately


## Messages

JSON messages have defined to indicate requests and results of files transfer

* Get file request message (from client to server)
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


* Post file request message (from client to server, or verse)
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


* Response (from server to client)
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
