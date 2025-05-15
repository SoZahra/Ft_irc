#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>     // Pour les entrées/sorties standards
# include <string>       // Pour manipuler les chaînes de caractères
# include <map>          // Pour stocker les clients et les canaux
# include <vector>       // Pour stocker les collections de données
# include <ctime>        // Pour horodater les messages et les événements
# include <cstring>      // Pour les fonctions de manipulation de chaînes C
# include <cerrno>       // Pour les codes d'erreur
# include <sys/socket.h> // Pour les fonctions socket
# include <netinet/in.h> // Pour les structures de socket
# include <arpa/inet.h>  // Pour les conversions d'adresses
# include <fcntl.h>      // Pour fcntl (mode non-bloquant)
# include <poll.h>       // Pour poll
# include <unistd.h>     // Pour close, etc.
# include <sstream>      // Pour la manipulation des flux de chaînes
# include <csignal>      // Pour la gestion des signaux

# include "Client.hpp"
# include "Channel.hpp"
# include "CommandHandler.hpp"
# include "FileTransfer.hpp" // Pour les bonus - transfert de fichiers
# include "Bot.hpp"          // Pour les bonus - bot IRC

// Nombre maximum de clients que le serveur peut gérer
# define MAX_CLIENTS 100
// Taille maximum du buffer pour la réception des données
# define BUFFER_SIZE 1024

class Client;
class Channel;
class CommandHandler;
class FileTransfer;
class Bot;

class Server
{
private:
	int                         _serverSocket;       // Socket principal du serveur
	int                         _port;               // Port d'écoute du serveur
	std::string                 _password;           // Mot de passe pour se connecter au serveur
	std::string                 _serverName;         // Nom du serveur IRC
	std::string                 _creationDate;       // Date de création du serveur
	std::map<int, Client*>      _clients;            // Map des clients connectés (fd → Client)
	std::map<std::string, Channel*> _channels;       // Map des canaux existants (nom → Channel)
	struct pollfd               _fds[MAX_CLIENTS+1]; // Tableau pour poll (socket serveur + clients)
	int                         _nfds;               // Nombre de descripteurs suivis par poll
	CommandHandler*             _commandHandler;     // Gestionnaire de commandes
	bool                        _running;            // État d'exécution du serveur

	// Bonus
	FileTransfer*               _fileTransfer;       // Gestionnaire de transfert de fichiers
	Bot*                        _bot;                // Bot IRC

	// Méthodes privées utilisées en interne par le serveur
	void setupServerSocket();                        // Configuration du socket serveur
	void acceptNewConnection();                      // Acceptation d'une nouvelle connexion
	void handleClientMessage(int clientFd);          // Traitement des messages des clients

public:
	// Constructeur et destructeur
	Server(int port, const std::string& password);
	~Server();

	// Méthodes principales
	void start();                                    // Démarrage du serveur
	void stop();                                     // Arrêt du serveur
	void removeClient(int clientFd);                 // Suppression d'un client

	// Getters
	std::string getPassword() const;
	std::string getServerName() const;
	std::string getCreationDate() const;

	// Gestion des clients
	Client* getClient(int fd) const;
	Client* getClientByNickname(const std::string& nickname) const;
	void broadcast(const std::string& message, int excludeFd = -1);
	unsigned int getClientCount() const;             // Nombre de clients connectés

	// Gestion des canaux
	Channel* getChannel(const std::string& name) const;
	Channel* createChannel(const std::string& name, Client* creator);
	void removeChannel(const std::string& name);
	std::map<std::string, Channel*> getChannels() const;
	unsigned int getChannelCount() const;            // Nombre de canaux existants

	// Bonus - Transfert de fichiers
	void initFileTransfer();
	void handleFileTransfer(Client* sender, Client* receiver, const std::string& filename);

	// Bonus - Bot
	void initBot();
	void handleBotCommand(Client* client, const std::string& command, const std::string& params);
};

#endif