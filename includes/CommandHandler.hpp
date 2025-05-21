#ifndef COMMAND_HANDLER_HPP
# define COMMAND_HANDLER_HPP

# include <string>       // Pour les chaînes de caractères
# include <map>          // Pour stocker les commandes
# include <vector>       // Pour stocker les paramètres

# include "Server.hpp"
# include "Client.hpp"
# include "Command.hpp"

class Server;
class Client;
class Command;

// Réponses numériques standardisées IRC
enum IRCReply {
    RPL_WELCOME = 001,              // Bienvenue sur le serveur IRC
    RPL_YOURHOST = 002,             // Votre hôte est X, exécutant la version Y
    RPL_CREATED = 003,              // Ce serveur a été créé le...
    RPL_MYINFO = 004,               // <servername> <version> <available user modes> <available channel modes>
    RPL_UMODEIS = 221,              // Mode utilisateur
    RPL_LUSERCLIENT = 251,          // Il y a <x> utilisateurs et <y> invisibles sur <z> serveurs
    RPL_LUSEROP = 252,              // <integer> :operator(s) online
    RPL_LUSERUNKNOWN = 253,         // <integer> :unknown connection(s)
    RPL_LUSERCHANNELS = 254,        // <integer> :channels formed
    RPL_LUSERME = 255,              // :I have <integer> clients and <integer> servers
    RPL_AWAY = 301,                 // <nick> :<away message>
    RPL_UNAWAY = 305,               // :You are no longer marked as being away
    RPL_NOWAWAY = 306,              // :You have been marked as being away
    RPL_WHOISUSER = 311,            // <nick> <user> <host> * :<real name>
    RPL_WHOISSERVER = 312,          // <nick> <server> :<server info>
    RPL_WHOISOPERATOR = 313,        // <nick> :is an IRC operator
    RPL_WHOISIDLE = 317,            // <nick> <integer> :seconds idle
    RPL_ENDOFWHOIS = 318,           // <nick> :End of WHOIS list
    RPL_WHOISCHANNELS = 319,        // <nick> :*( ( "@" / "+" ) <channel> " " )
    RPL_CHANNELMODEIS = 324,        // <channel> <mode> <mode params>
    RPL_NOTOPIC = 331,              // <channel> :No topic is set
    RPL_TOPIC = 332,                // <channel> :<topic>
    RPL_TOPICWHOTIME = 333,         // <channel> <nick> <time>
    RPL_INVITING = 341,             // <channel> <nick>
    RPL_NAMREPLY = 353,             // <channel> :<list of nicks>
    RPL_ENDOFNAMES = 366,           // <channel> :End of NAMES list
    RPL_BANLIST = 367,              // <channel> <banmask>
    RPL_ENDOFBANLIST = 368,         // <channel> :End of channel ban list
    RPL_YOUREOPER = 381             // :You are now an IRC operator
};

// Erreurs standardisées IRC
enum IRCError {
    ERR_NOSUCHNICK = 401,           // <nickname> :No such nick/channel
    ERR_NOSUCHSERVER = 402,         // <server name> :No such server
    ERR_NOSUCHCHANNEL = 403,        // <channel name> :No such channel
    ERR_CANNOTSENDTOCHAN = 404,     // <channel name> :Cannot send to channel
    ERR_TOOMANYCHANNELS = 405,      // <channel name> :You have joined too many channels
    ERR_WASNOSUCHNICK = 406,        // <nickname> :There was no such nickname
    ERR_TOOMANYTARGETS = 407,       // <target> :Duplicate recipients. No message delivered
    ERR_NOORIGIN = 409,             // :No origin specified
    ERR_NORECIPIENT = 411,          // :No recipient given (<command>)
    ERR_NOTEXTTOSEND = 412,         // :No text to send
    ERR_UNKNOWNCOMMAND = 421,       // <command> :Unknown command
    ERR_NOMOTD = 422,               // :MOTD File is missing
    ERR_NONICKNAMEGIVEN = 431,      // :No nickname given
    ERR_ERRONEUSNICKNAME = 432,     // <nick> :Erroneous nickname
    ERR_NICKNAMEINUSE = 433,        // <nick> :Nickname is already in use
    ERR_USERNOTINCHANNEL = 441,     // <nick> <channel> :They aren't on that channel
    ERR_NOTONCHANNEL = 442,         // <channel> :You're not on that channel
    ERR_USERONCHANNEL = 443,        // <user> <channel> :is already on channel
    ERR_NOTREGISTERED = 451,        // :You have not registered
    ERR_NEEDMOREPARAMS = 461,       // <command> :Not enough parameters
    ERR_ALREADYREGISTERED = 462,    // :You may not reregister
    ERR_PASSWDMISMATCH = 464,       // :Password incorrect
    ERR_CHANNELISFULL = 471,        // <channel> :Cannot join channel (+l)
    ERR_UNKNOWNMODE = 472,          // <char> :is unknown mode char to me
    ERR_INVITEONLYCHAN = 473,       // <channel> :Cannot join channel (+i)
    ERR_BANNEDFROMCHAN = 474,       // <channel> :Cannot join channel (+b)
    ERR_BADCHANNELKEY = 475,        // <channel> :Cannot join channel (+k)
    ERR_BADCHANMASK = 476,          // <channel> :Bad Channel Mask
    ERR_CHANOPRIVSNEEDED = 482,     // <channel> :You're not channel operator
    ERR_UMODEUNKNOWNFLAG = 501,     // :Unknown MODE flag
    ERR_USERSDONTMATCH = 502        // :Cannot change mode for other users
};

class CommandHandler
{
private:
	Server*                 _server;        // Pointeur vers le serveur
	std::map<std::string, Command*> _commands; // Map des commandes disponibles

	// Méthodes privées
	void registerCommands();                 // Enregistrement des commandes
	std::vector<std::string> parseParams(const std::string& paramsStr); // Parse les paramètres

public:
	// Constructeur et destructeur
	CommandHandler(Server* server);
	~CommandHandler();

	// Exécution de commandes
	void executeCommand(Client* client, const std::string& message);

	// Formatage des réponses
	std::string formatReply(int code, Client* client, const std::string& message);
	std::string formatReply(int code, Client* client, const std::vector<std::string>& params);

	// Méthodes utilitaires
	bool isValidChannelName(const std::string& name);
	bool isValidNickname(const std::string& nickname);

	bool isCommandValid(const std::string& cmdName) const;
};

#endif