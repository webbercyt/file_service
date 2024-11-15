# Server-Client File Exchange 
> Server-client implementation of [RFC 6455 websocket protocol](https://www.rfc-editor.org/rfc/rfc6455) to file exchange 

A uint64_t flag, [max_clients](https://github.com/webbercyt/file_service/blob/master/utils/resource.h), controls the number of allowed workers/clients to server.

![](infrastructure.png)

Please see details in [C4 diagrams](https://github.com/webbercyt/file_service/blob/master/diagrams.pdf).


## Development setup

1. Dependencies

   c++17

   boost-v1.86.0 (minimum dependencies of boost are under ./third_party, ideally no more dependency setup required.)

3. Build

   use cmake to build ./file_server and ./file_client


## Server usage example

* To start the server, run the command: _file_server {host} {port} {threads} {dir}_

  	* _{host}_: ip address or domain name of the server

   	* _{port}_: port number the server is listening to

  	* _{threads}_: number of thread the I/O service running on

  	* _{dir}_: directory in server endpoint where files are read from or written to

	example: 
	> file_server 0.0.0.0 80 4 /usr/files/server
	
	
* To shut down the server, use the 'x' command
	>x


## Client usage example

* To start the client, run the command: _file_client {host} {port} {dir}_

	* _{host}_: ip address or domain name of the server

  	* _{port}_: port number the server is listening to

  	* _{dir}_: directory in client endpoint where files are read from or written to

	example: 
	> file_client 192.168.0.1 80 /usr/files/client
	
	
To shut down the client, use the 'x' command
	>x


* To retrive file(s) from server, use the 'get' command: _get -{scope=a|s} {file_name}_ 
	
   {scope} = a : get all files under server directory
   
   {scope} = s : get a singel file under server directory

	examples: 
	>get -a
	
	or
	>get -s server-client.png


* To send file(s) to server, run 'post' command: _post -{scope=a|s} {file_name}_

	
   {scope} = a : post all files under client directory
   
   {scope} = s : post a singel file under server directory
   
	examples:
	>post -a
	
	or
	>post -s client-server.png


## Messages

JSON messages have been defined to indicate requests and responses

* Get file request message (from client to server)
   
   * _"uuid"_: unique message id, used to trace response
   
   * _"method"_: "get"
   
   * _"scope"_: "single" = single file; "all" = all valid files
   
   * _"target"_: file name, e.g. get_request_target.txt

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

   * _"method"_: "get"

   * _"target"_: file name, e.g. get_request_target.txt

   * _"context"_: binary encoded context of file

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
   
   * _"uuid"_: unique message id, used to trace request
   
   * _"response"_: "accepted" or "rejected"
   
   * _"reason"_ (optional): interpretation of response 

	```
	Examples
	{
		"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
		"response":"rejected"
		"reason":"failed to parse message"
	}
	```


## Todo 

* Handle errors in UTF-8 encoded data

Based on RFC 6455, when an endpoint is to interpret a byte stream as UTF-8 (e.g. file name) but finds that the byte stream is not, in fact, a valid UTF-8 stream, that endpoint fails the webSocket connection. What shall be further done is to handle non-UTF-8 stream properly, a potential solution is to introduce wide character upon file manipulations. 
