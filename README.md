# ft_irc
Our own IRC Server written in C++, based on the RFC protocol.

> Made by: [🦞 pruiz-ca](https://github.com/pruiz-ca) [🦔 rorozco-](https://github.com/larroky) [🐨 mmunoz-f ](https://github.com/mmunoz-f)

## How to use

Compile the repository with `make`, then run `ircserv <port> <server_password>`, after that you will be able to connect from any client which follows RFC protocol.

Our server has the basics implementations, letting us comunicate, create channels and modifying them.
To log in propperly:

```bash
	PASS <server_password>
	USER <username> 0 * :<real_name>
	NICK <nick_name>
```