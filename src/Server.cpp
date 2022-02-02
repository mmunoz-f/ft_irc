/*		¯\_(ツ)_/¯		*/

#include "Channel.hpp"
#include "Server.hpp"
#include "User.hpp"
#include "utils.hpp"
#include "ModeFlag.hpp"

Server::Server() : terminate(false) { killSwitch = &terminate; }
Server::~Server() {
	for (pollfd_vct::iterator it = poll_fds.begin(); it != poll_fds.end(); it++)
		if (it->fd != server_fd)
			close(it->fd);
	close(server_fd);
	for (Channel::flag_map::const_iterator it = Channel::flags.begin(); it != Channel::flags.end(); it++)
		delete it->second;
}
Server::Server(const Server& other) { *this = other; }

Server&		Server::operator=(const Server& other) {
	if (this != &other) {
		poll_fds = other.poll_fds;
		users_fd = other.users_fd;
		users_nick = other.users_nick;
		channels = other.channels;
		commands = other.commands;
		server_size = other.server_size;
		conn_size = other.conn_size;
		server_addr = other.server_addr;
		conn_addr = other.conn_addr;
		hostname = other.hostname;
		server_fd = other.server_fd;
		server_port = other.server_port;
		op_credentials = other.op_credentials;
	}
	return *this;
}

int		Server::check(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	if (std::strlen(argv[2]) < 2) {
		std::cerr << "Password must have at least 2 characters" << std::endl;
		return 1;
	}
	for (size_t i = 0; i < std::strlen(argv[2]); i++) {
		if (!std::isprint(argv[2][i])) {
			std::cerr << "[Error]: password must have only printable characters" << std::endl;
			return 1;
		}
	}
	for (size_t i = 0; i < std::strlen(argv[1]); i++) {
		if (!(argv[1][i] >= '0' && argv[1][i] <= '9')) {
			std::cerr << "[Error]: Invalid port: must contain only numbers" << std::endl;
			return 1;
		}
	}
	server_passwd = argv[2];
	server_port = std::atoi(argv[1]);
	if (server_port <= 1000 || server_port > 65535) {
		std::cerr << "[Error]: Invalid port range: 1000 < port <= 65535" << std::endl;
		return 1;
	}
	return 0;
}

void	Server::init() {
	loadCommandMap();
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(server_port);
	server_size = sizeof(server_addr);
	conn_size = sizeof(conn_addr);
	hostname = getServerIP();

	op_credentials["admin"] = "admin";

	memset(buffer, 0, BUFF_SIZE);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "[Server]: Socket couldn't be created" << std::endl;
		std::exit(-1);
	}

	struct pollfd	server_poll_fd = (struct pollfd){server_fd, POLLIN, 0};
	poll_fds.push_back(server_poll_fd);

	int		yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(server_fd, (sockaddr*)&server_addr, server_size) < 0) {
		std::cerr << "[Server]: Socket bind failed" << " " << strerror(errno) << std::endl;
		std::exit(-1);
	}
}

void	Server::start() {
	if (listen(server_fd, BACKLOG) < 0) {
		std::cerr << "[Server]: Socket listen failed" << std::endl;
		std::exit(-1);
	}
	std::cout << BLU << "[Server]: IRC Server started at " << hostname
		<< ":" << server_port << NCL << std::endl;
	time_t		current_time;
	time(&current_time);
	timestamp_creation = ctime(&current_time);
	timestamp_creation.resize(timestamp_creation.size() - 1);
	while (!terminate) {
		utils::recSignals();
		poll(poll_fds.begin().base(), (nfds_t)poll_fds.size(), POLL_TIMEOUT_MS);
		for (pollfd_vct::iterator it = poll_fds.begin(); it < poll_fds.end(); it++) {
			if (it->revents & POLLHUP)
				removeClient(it);
			else if (it->revents & POLLIN) {
				if (it->fd == server_fd)
					addClient(it);
				else {
					updateTimestamp(it->fd);
					getMsg(it);
					handleMsg(it);
				}
			}
			else if (it->revents & POLLOUT)
				if (send_leftovers.count(it->fd))
					sendCustom(it->fd, send_leftovers[it->fd], send_leftovers[it->fd].size(), 0);
		}
		pingloop();
	}
	std::cout << BLU << "[Server]: Bye bye!" << std::endl;
}

void	Server::pingloop() {
	for (pollfd_vct::iterator vit = poll_fds.begin(); vit != poll_fds.end(); vit++) {
		if (vit->fd != server_fd) {
			User*		u = &users_fd[vit->fd];
			if (u->ping_request && std::time(NULL) - u->timestamp > PINGPONG_TIMEOUT_S) {
				pollfd_vct::iterator tmp = vit--;
				sendReply(tmp->fd, hostname, "ERROR [Timeout]");
				removeClient(tmp);
			}
			else if (!u->ping_request && std::time(NULL) - u->timestamp > PINGPONG_FREQ_S) {
				u->ping_key = utils::randomStringGenerator(7);
				u->ping_request = true;
				u->timestamp = std::time(NULL);
				sendReply(vit->fd, "", "PING :" + u->ping_key);
			}
		}
	}
}

void	Server::addClient(pollfd_vct::iterator &it) {
	int							conn_fd;
	struct pollfd				conn_poll_fd;
	pollfd_vct::difference_type	it_difference;

	if ((conn_fd = accept(poll_fds.begin()->fd, (sockaddr*)&conn_addr, &conn_size)) < 0) {
		std::cerr << "[Server]: Connection was not accepted" << std::endl;
		std::exit(-1);
	}
	conn_poll_fd = (struct pollfd){conn_fd, POLLIN, 0};
	it_difference = std::distance(poll_fds.begin(), it);

	fcntl(conn_fd, F_SETFL, O_NONBLOCK);
	poll_fds.push_back(conn_poll_fd);
	it = poll_fds.begin() + it_difference;

	User	user(conn_fd, conn_addr.sin_addr, this);
	users_fd.insert(std::make_pair(conn_fd, user));
	users_fd[conn_fd].timestamp = std::time(NULL);
	users_fd[conn_fd].ping_request = true;

	std::cout << BLU << "[Server]: Client " << conn_fd
		<< " from " << inet_ntoa(conn_addr.sin_addr)
		<< ":" << ntohs(conn_addr.sin_port)
		<< " connected" << NCL << std::endl;
}

void	Server::removeClient(pollfd_vct::iterator &it) {
	users_fd[it->fd].serveraccess = false;
	users_fd[it->fd].leaveChannel();
	for (User::channel_map::const_iterator i = users_fd[it->fd].channels.begin(); i != users_fd[it->fd].channels.end(); i++)
		if (i->second->empty()) {
			channels.erase((i->second->name));
		}
	users_nick.erase((users_fd[it->fd].nickname));
	users_fd.erase(it->fd);
	send_leftovers.erase(it->fd);
	recv_leftovers.erase(it->fd);

	std::cout << RED << "[Server]: Client " << it->fd
		<< " from " << inet_ntoa(conn_addr.sin_addr)
		<< ":" << ntohs(conn_addr.sin_port)
		<< " disconnected" << NCL << std::endl;

	close(it->fd);
	poll_fds.erase(it);
}

void	Server::validateUser(const int &fd, const Message &cmd, User &user) {
	if (!user.nickname.empty() && !user.username.empty() && !(user.mode & VALID_USER)) {
		user.mode |= VALID_USER;
		user.timestamp = std::time(NULL);
		user.timestamp_login = std::time(NULL);
		user.ping_request = false;
		sendReply(fd, hostname, RPL_WELCOME + string(" ") + user.nickname + string(" ") + RPL_WELCOME_MSG + " " + user.getFullIdentifier());
		sendReply(fd, hostname, RPL_YOURHOST + string(" ") + user.nickname + string(" ") + RPL_YOURHOST_MSG);
		sendReply(fd, hostname, RPL_CREATED + string(" ") + user.nickname + string(" ") + RPL_CREATED_MSG + " " + timestamp_creation);
		sendReply(fd, hostname, RPL_MYINFO + string(" ") + user.nickname + string(" ") + RPL_MYINFO_MSG);
		sendReply(fd, hostname, RPL_ISUPPORT + string(" ") + user.nickname + string(" ") + RPL_ISUPPORT_MSG);
		motd(fd, cmd);
	}
}

void	Server::getMsg(pollfd_vct::iterator &it) {
	receivedMsg.clear();
	memset(&buffer, 0, BUFF_SIZE);
	if (recv(it->fd, buffer, BUFF_SIZE, 0) == 0)
		return removeClient(it);
	if (DEBUG) {
		std::stringstream	fd_str;
		fd_str << it->fd;
		std::cout << PRP << "[Rcvd from " + fd_str.str() + "] // " << buffer << NCL;
	}
	if (buffer[std::strlen(buffer) - 1] == '\n') {
		if (recv_leftovers.count(it->fd) > 0) {
			receivedMsg = recv_leftovers[it->fd];
			recv_leftovers.erase(it->fd);
		}
		receivedMsg.append(buffer);
	}
	else {
		if (recv_leftovers.count(it->fd) > 0)
			recv_leftovers[it->fd].append(buffer);
		else
			recv_leftovers[it->fd] = buffer;
	}
}

void	Server::handleMsg(pollfd_vct::iterator &it) {
	vector<string>		tmp = utils::strsplit(receivedMsg, '\n');

	for (vector<string>::iterator vit = tmp.begin(); vit != tmp.end(); vit++) {
		Message		cmd(*vit);
		command_map::iterator mit = commands.find(cmd.name);
		if (mit != commands.end()) {
			if ((cmd.name == "PASS" || cmd.name == "NICK" || cmd.name == "USER" || cmd.name == "QUIT")
				|| users_fd[it->fd].mode & VALID_USER) {
				ptr tmp = (*mit).second;
				(this->*tmp)(it->fd, cmd);
			}
			continue ;
		}
		if (!cmd.name.empty() && users_fd[it->fd].mode & VALID_USER) {
			string	cmdname = cmd.full.substr(0, cmd.full.find(' '));
			sendReply(it->fd, hostname, string(ERR_UNKNOWNCOMMAND) + " " + users_fd[it->fd].nickname + " " + (cmdname) + " " + ERR_UNKNOWNCOMMAND_MSG);
		}
	}
}

string		Server::getServerIP() {
	char				tmp_hostname[HOST_NAME_MAX];
	struct	hostent*	hostentry;

	gethostname(tmp_hostname, HOST_NAME_MAX);
	hostentry = gethostbyname(tmp_hostname);
	if (!hostentry)
		return string("127.0.0.1");
	return string(inet_ntoa(*((struct in_addr*)hostentry->h_addr_list[0])));
}

void Server::loadCommandMap() {
	commands["WHO"] = &Server::who;
	commands["PASS"] = &Server::pass;
	commands["NICK"] = &Server::nick;
	commands["USER"] = &Server::user;
	commands["PING"] = &Server::ping;
	commands["PONG"] = &Server::pong;
	commands["JOIN"] = &Server::join;
	commands["PART"] = &Server::part;
	commands["OPER"] = &Server::oper;
	commands["MODE"] = &Server::mode;
	commands["LIST"] = &Server::list;
	commands["QUIT"] = &Server::quit;
	commands["MOTD"] = &Server::motd;
	commands["KICK"] = &Server::kick;
	commands["ISON"] = &Server::ison;
	commands["KILL"] = &Server::kill;
	commands["NAMES"] = &Server::names;
	commands["WHOIS"] = &Server::whois;
	commands["TOPIC"] = &Server::topic;
	commands["SQUIT"] = &Server::squit;
	commands["NOTICE"] = &Server::privmsg;
	commands["PRIVMSG"] = &Server::privmsg;
}

void	Server::sendReplyToAll(const string &prefix, const string &reply) {
	for (userfd_map::iterator it = users_fd.begin(); it != users_fd.end(); it++)
		sendReply(it->first, prefix, reply);
}

void	Server::sendReply(const int &fd, const string &prefix, const string &reply) {
	if (fd < 0)
		return ;
	string tmp = ":" + prefix + " " + reply + "\r\n";
	sendCustom(fd, tmp, tmp.length(), 0);
}

void	Server::sendCustom(const int &fd, const string& msg, size_t size, int flags) {
	size_t		sent = send(fd, &msg[0], size, flags);
	if (DEBUG) {
		std::stringstream	fd_str;
		fd_str << fd;
		std::cout << PNK << "[Sent to " + fd_str.str() + "] // " << msg << NCL;
	}
	if (sent < size)
		send_leftovers.insert(std::make_pair(fd, msg.substr(sent, size)));
}

void	Server::sendReplyToChannel(Channel &channel, const string &prefix, const string &reply, const int &self) {
	for (Channel::userfd_map::iterator it = channel.users_fd.begin(); it != channel.users_fd.end(); it++) {
		if (it->first != self)
			sendReply(it->first, prefix, reply);
	}
}

void	Server::updateTimestamp(const int &fd) {
	if (users_fd[fd].mode & VALID_USER) {
		users_fd[fd].timestamp = std::time(NULL);
		users_fd[fd].ping_request = false;
	}
}

void	Server::setTerminate(bool value) { terminate = value; }
