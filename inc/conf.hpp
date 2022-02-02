/*		／人◕ __ ◕人＼		*/

#pragma once

#ifndef DEBUG
# define DEBUG 0
#endif

#define BACKLOG 5
#define BUFF_SIZE 512

#ifndef HOST_NAME_MAX
# define HOST_NAME_MAX 64
#endif

#define PINGPONG_FREQ_S 120
#define PINGPONG_TIMEOUT_S 30
#define POLL_TIMEOUT_MS 3000

#define NCL "\033[0m"
#define BLU "\033[0;36m"
#define GRN "\033[0;32m"
#define RED "\033[0;31m"
#define PNK "\033[0;35m"
#define PRP "\033[0;34m"

/* Channel Flags */

/* Channel user modes */
#define C_O			0b1000
#define C_V			0b0010

/* Channel modes */
#define C_K			0b000010
#define C_M			0b001000
#define C_N			0b010000
#define C_T			0b000001
#define C_B         0b100000

/* Global user modes */
#define VALID_USER	0b1000
#define S_OP		0b0100

/* Reply codes */
#define RPL_WELCOME "001"
#define RPL_WELCOME_MSG ":Welcome to the Internet Relay Network"

#define RPL_YOURHOST "002"
#define RPL_YOURHOST_MSG ":Your host is ircserv, running version 1.0"

#define RPL_CREATED "003"
#define RPL_CREATED_MSG ":This server was created"

#define RPL_MYINFO "004"
#define RPL_MYINFO_MSG ":ircserv 1.0 ovb mt"

#define RPL_ISUPPORT "005"
#define RPL_ISUPPORT_MSG ":CASEMAPPING=<ascii> CHANMODES=A,B,C,D HOSTLEN=64 NICKLEN=9 :are supported by this server"

#define RPL_UMODEIS "221"
#define RPL_UMODEIS_MSG "your user mode is"

#define RPL_ISON "301"

#define RPL_WHOIS "311"
#define RPL_WHOISOPERATOR "313"
#define RPL_WHOISIDLE "317"
#define RPL_ENDOFWHOIS "318"
#define RPL_ENDOFWHOIS_MSG "End of /WHOIS list."
#define RPL_WHOISCHANNELS "319"

#define RPL_LIST_HEAD "321"

#define RPL_LIST "322"

#define RPL_LISTEND "323"
#define RPL_LISTEND_MSG ":End of /LIST"

#define RPL_CHANNELMODEIS "324"
#define RPL_CHANNELMODEIS_MSG ":Mode is"

#define RPL_NOTOPIC "331"
#define RPL_NOTOPIC_MSG ":No topic is set"

#define RPL_TOPIC "332"
#define RPL_TOPIC_MSG ":"

#define RPL_ENDOFWHO "315"
#define RPL_ENDOFWHO_MSG ":End of /WHO list."
#define RPL_WHOREPLY "352"

#define RPL_NAMREPLY "353"

#define RPL_ENDOFNAMES "366"
#define RPL_ENDOFNAMES_MSG ":End of /NAMES list."

#define RPL_BANLIST "367"

#define RPL_ENDOFBANLIST "368"
#define RPL_ENDOFBANLIST_MSG ":End of channel ban list"

#define RPL_MOTD "372"

#define RPL_MOTDSTART "375"
#define RPL_MOTDSTART_MSG ":IRC Server message of the day"

#define RPL_ENDOFMOTD "376"
#define RPL_ENDOFMOTD_MSG ":End of /MOTD command."

#define RPL_YOUREOPER "381"
#define RPL_YOUREOPER_MSG ":You are now an IRC operator"

#define RPL_DEFAULTKICK_MSG "Misbehaving badly ;)"

/* Error codes */
#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHNICK_MSG ":No such nick/channel"

#define ERR_NOSUCHCHANNEL "403"
#define ERR_NOSUCHCHANNEL_MSG ":No such channel"

#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_CANNOTSENDTOCHAN_MSG ":Cannot send to channel"

#define ERR_NOTEXTTOSEND "412"
#define ERR_NOTEXTTOSEND_MSG ":No text to send"

#define ERR_UNKNOWNCOMMAND "421"
#define ERR_UNKNOWNCOMMAND_MSG ":Unknown command"

#define ERR_NONICKNAMEGIVEN "431"
#define ERR_NONICKNAMEGIVEN_MSG ":No nickname given"

#define ERR_ERRONEUSNICKNAME "432"
#define ERR_ERRONEUSNICKNAME_MSG ":Erroneous Nickname"

#define ERR_NICKNAMEINUSE "433"
#define ERR_NICKNAMEINUSE_MSG ":Nickname is already in use"

#define ERR_USERNOTINCHANNEL "441"
#define ERR_USERNOTINCHANNEL_MSG ":They aren't on that channel"

#define ERR_NOTONCHANNEL "442"
#define ERR_NOTONCHANNEL_MSG ":You're not on that channel"

#define ERR_USERONCHANNEL "443"
#define ERR_USERONCHANNEL_MSG ":is already on channel"

#define ERR_NEEDMOREPARAMS "461"
#define ERR_NEEDMOREPARAMS_MSG(comm) #comm " :Not enough parameters."

#define ERR_ALREADYREGISTERED "462"
#define ERR_ALREADYREGISTERED_MSG ":Unauthorized command (already registered)"

#define ERR_PASSWDMISMATCH "464"
#define ERR_PASSWDMISMATCH_MSG ":Password incorrect"

#define ERR_UNKNOWNMODE "472"
#define ERR_UNKNOWNMODE_MSG ":is unknown mode char to me"

#define ERR_BANNEDFROMCHAN "474"
#define ERR_BANNEDFROMCHAN_MSG ":Cannot join channel (+b)"

#define ERR_BADCHANMASK "476"
#define ERR_BADCHANMASK_MSG ":Bad Channel Mask"

#define ERR_NOPRIVILEGES "481"
#define ERR_NOPRIVILEGES_MSG ":Permission Denied- You're not an IRC operator"

#define ERR_CHANOPRIVSNEEDED "482"
#define ERR_CHANOPRIVSNEEDED_MSG ":You're not channel operator"

#define ERR_UMODEUNKNOWNFLAG "501"
#define ERR_UMODEUNKNOWNFLAG_MSG ":Unknown MODE flag"

#define ERR_USERSDONTMATCH "502"
#define ERR_USERSDONTMATCH_MSG  ":Cant change mode for other users"
#define ERR_USERSDONTMATCH_SEE_MSG ":Can't view modes for other users"

#define ERR_INVALIDMODEPARAM "696"
#define ERR_BADPARAMUSE_MSG ":Wrong parameters use"

/* ban list rpls */
#define ERR_NOTONBANLIST_MSG ":Channel ban list does not contain"
#define ERR_ALREADYONBANLIST_MSG ":Channel ban list already contains"
#define RPL_BANSET_MSG "sets ban on"
#define RPL_BANUNSET_MSG "removes ban on"

#define RPL_SETVOICE_MSG "gives voice to"
#define RPL_UNSETVOICE_MSG "removes voice from"

#define RPL_SETOP_MSG "gives channel operator status to"
#define RPL_UNSETOP_MSG "removes channel operator status from"