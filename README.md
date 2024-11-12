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

get: get -{scope:a|s} {file_name}

get all example: 
>get -a

get single example: 
>get -s server-client.png


post: post -{scope:a|s} {file_name}

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
   * uuid[string]: unique message id, used to trace response
   * method[string]: "get"
   * scope[string]: "single" = single file; "all" = all valid files
   * target[string]: file name, e.g. get_request_target.txt
 ```
{
	"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
	"method":"get"
	"scope":"single"|"all"
	"target":{file_name}
}
 ```

* post file request: from client to server
    * CHANGE: Remove `setDefaultXYZ()`
    * ADD: Add `init()`
* 0.1.1
    * FIX: Crash when calling `baz()` (Thanks @GenerousContributorName!)
* 0.1.0
    * The first proper release
    * CHANGE: Rename `foo()` to `bar()`
* 0.0.1
    * Work in progress

## Meta

Your Name – [@YourTwitter](https://twitter.com/dbader_org) – YourEmail@example.com

Distributed under the XYZ license. See ``LICENSE`` for more information.

[https://github.com/yourname/github-link](https://github.com/dbader/)

## Contributing

1. Fork it (<https://github.com/yourname/yourproject/fork>)
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request

<!-- Markdown link & img dfn's -->
[npm-image]: https://img.shields.io/npm/v/datadog-metrics.svg?style=flat-square
[npm-url]: https://npmjs.org/package/datadog-metrics
[npm-downloads]: https://img.shields.io/npm/dm/datadog-metrics.svg?style=flat-square
[travis-image]: https://img.shields.io/travis/dbader/node-datadog-metrics/master.svg?style=flat-square
[travis-url]: https://travis-ci.org/dbader/node-datadog-metrics
[wiki]: https://github.com/yourname/yourproject/wiki
