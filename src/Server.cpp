#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Utils.hpp"
#include <unistd.h>  // Pour close() et autres fonctions POSIX
#include <cstring>   // Pour strerror


Server::Server(int port, const std::string& password):
    _serverSocket(-1),	// Socket serveur non initialisé
    _port(port),	// Port d'écoute du serveur
    _password(password),	// Mot de passe pour se connecter au serveur
    _serverName("ft_irc"),	// Nom par défaut du serveur IRC
    _creationDate(Utils::getCurrentTime()),	// Date de création du serveur
    _nfds(1),	// Nombre de descripteurs suivis par poll
    _commandHandler(NULL),
	_running(false), // État d'exécution du serveur
    _fileTransfer(NULL),
    _bot(NULL)	// Pointeur vers le bot IRC

{
	memset(_fds, 0, sizeof(_fds));	// Initialiser le tableau de descripteurs à zéro
	_commandHandler  = new CommandHandler(this);	// Créer le gestionnaire de commandes

	initFileTransfer();	// Initialiser le gestionnaire de transfert de fichiers
	initBot();	// Initialiser le bot IRC

	Utils::logMessage("Serveur IRC cree sur le port " + Utils::toString(port) + " avec le mot de passe");	// Log de création du serveur
}

Server::~Server(){

	if(_running){
		stop();
	}
	// Nettoyer les clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
	//les canaux
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	// gestionnaire de commandes
	if (_commandHandler)
	{
		delete _commandHandler;
		_commandHandler = NULL;
	}
	// bonus
	if (_fileTransfer)
	{
		delete _fileTransfer;
		_fileTransfer = NULL;
	}
	if (_bot)
	{
		delete _bot;
		_bot = NULL;
	}
	// Fermer le socket serveur s'il est ouvert
	if (_serverSocket != -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
	}
	Utils::logMessage("Serveur IRC détruit");
}

void Server::start(){
	if (_running)
		return;
	setupServerSocket();// Config socket du serveur

	// serveur en cours d'exec
	_running = true;
	Utils::logMessage("Serveur IRC démarré");

	while (_running)
	{
		// Attendre des événements sur les sockets avec poll
		int pollResult = poll(_fds, _nfds, 1000);  // Timeout de 1 seconde
		if (pollResult < 0)
		{
			if (errno == EINTR){
				continue;
			}
			else{
				throw std::runtime_error("Erreur lors de l'appel à poll: " + std::string(strerror(errno)));
			}
		}

		// Vérifier si poll a expiré sans événements
		if (pollResult == 0)
			continue;

		// Traiter les événements
		for (int i = 0; i < _nfds; ++i)
		{
			// Vérifier s'il y a un événement sur ce descripteur
			if (_fds[i].revents == 0){
				continue;
			}
			if (_fds[i].fd == _serverSocket){// Vérifier si c'est le socket serveur
				acceptNewConnection();// Nouvelle connexion
			}
			else{
				handleClientMessage(_fds[i].fd);// Msg du client
			}
			_fds[i].revents = 0;
		}
	}
	if (_serverSocket != -1)// Fermer le socket serveur
	{
		close(_serverSocket);
		_serverSocket = -1;
	}
	Utils::logMessage("Serveur IRC arrêté");
}


void Server::stop(){
	_running = false;	// Marquer le serveur comme arrêté
	Utils::logMessage("Arret du serveur IRC en cours ...");
}


void Server::removeClient(int clientFd){
	std::map<int, Client*>::iterator it = _clients.find(clientFd);	// Rechercher le client par son descripteur de fichier
	if(it == _clients.end()){
		return;
	}
	Client* client = it->second;	// Obtenir le client
	Utils::logMessage("Client deconnecte: " + client->toString());	// Log de déconnexion du client

	if(client->isRegistered() && !client->getNickname().empty()){
		broadcast(":" + client->getNickname() + " !"  + client->getUsername() + "@" +
					client->getHostname() + " QUIT :Connection closed", clientFd); }

		//quitter les canaux
		std::vector<Channel*> channels = client->getChannels();
		for(size_t i = 0; i < channels.size(); ++i){
			channels[i]->removeClient(client);	// Supprimer le client de chaque canal
			if(channels[i]->getClientCount() == 0){
				_channels.erase(Utils::toLower(channels[i]->getName()));	// Supprimer le canal s'il n'y a plus de clients
				delete channels[i];	// Supprimer le canal
		}
	}
	close(clientFd);
	_clients.erase(it);
	delete client;	// Supprimer le client

	for(int i = 0; i <_nfds; ++i){
		if(_fds[i].fd == clientFd){

			for(int j = i; j < _nfds - 1; ++i){
				_fds[j] = _fds[j + 1];	// Décaler les descripteurs
			}
			_nfds--;
			break;
		}
	}
}


std::string Server::getPassword() const{
	return _password;
}

std::string Server::getServerName() const{
	return _serverName;
}

std::string Server::getCreationDate() const{
	return _creationDate;
}


Client* Server::getClient(int fd) const{
	std::map<int, Client*>::const_iterator it = _clients.find(fd);	// Rechercher le client par son descripteur de fichier
	if(it != _clients.end()){
		return it->second;
	}
	return NULL;
}

Client* Server::getClientByNickname(const std::string& nickname) const{
	//parcourir tous les clients
	for(std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it){
		if(Utils::toLower(it->second->getNickname()) == Utils::toLower(nickname)){
			return it->second;//si le pseudo correspond, retourner le client
		}
	}
	return NULL;
}

void Server::broadcast(const std::string& message, int excludeFd){
	for(std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it){
		if(it->first != excludeFd && it->second->isRegistered()){
			it->second->sendMessage(message);	// Envoyer le message à tous les clients sauf celui qui l'a envoyé
		}
	}
	Utils::logMessage("Message broadcast: " + message);	// Log du message broadcast
}

unsigned int Server::getClientCount() const{
	return _clients.size();	// nombre de clients connectés
}

unsigned int Server::getChannelCount() const{
	return _channels.size();	// nombre de canaux existants
}

Channel* Server::getChannel(const std::string& name) const{
	std::map<std::string, Channel*>::const_iterator it = _channels.find(Utils::toLower(name));
	if(it != _channels.end()){
		return it->second;
	}
	return NULL;
}

Channel* Server::createChannel(const std::string& name, Client* creator){
	// Vérifier si le canal existe deja
	if(getChannel(name)){
		return getChannel(name);	// Le canal existe déjà
	}
	// Creer un nouveau canal
	Channel* newChannel = new Channel(name, creator);
	_channels[Utils::toLower(name)] = newChannel;	// Ajouter le canal à la map des canaux
	Utils::logMessage("Canal cree: " + name + " par " + creator->getNickname());	// Log de création du canal
	return newChannel;	// Retourner le nouveau canal
}

void Server::removeChannel(const std::string& name){
	std::map<std::string, Channel*>::iterator it = _channels.find(Utils::toLower(name));
	if(it != _channels.end()){
		Utils::logMessage("Canal supprime: " + name);
		delete it->second;
		_channels.erase(it); // Supprimer le canal de la map
	}
}


void Server::initFileTransfer(){
	if(!_fileTransfer){
		_fileTransfer = new FileTransfer();	// Créer le gestionnaire de transfert de fichiers
		Utils::logMessage("Gestionnaire de transfert de fichiers initialise");	// Log d'initialisation
	}
}

void Server::handleFileTransfer(Client* sender, Client* receiver, const std::string& filename){
	if(!_fileTransfer){
		if(sender){
			sender->sendMessage(":" + _serverName + " Notice " + sender->getNickname() +
								" :Erreur: le systeme de transfert de fichiers n'est pas actif");
		}
		return;
	}
	if(!sender || !receiver){
		if(sender){
			sender->sendMessage(":" + _serverName + " Notice " + sender->getNickname() +
								" :Erreur: destinataire invalide");
		}
		return;
	}
	if(!Utils::fileExists(filename)){
		sender->sendMessage(":" + _serverName + " Notice " + sender->getNickname() +
							" :Erreur: le fichier n'existe pas" + filename);
		return;
	}
	size_t fileSize = Utils::getFileSize(filename);
	if(_fileTransfer->initTransfer(sender, receiver, filename, fileSize)){
		sender->sendMessage(":" + _serverName + " Notice " + sender->getNickname() +
							" :Transfert de fichier demarre vers " + filename + " vers " +
							receiver->getNickname() + " (" + Utils::toString(fileSize));

		receiver->sendMessage(":" + _serverName + " Notice " + receiver->getNickname() +
							":" + sender->getNickname() + " vous envoie un fichier: " +
							filename + " (" + Utils::toString(fileSize) + " octets)");
	}
	else{
		sender->sendMessage(":" + _serverName + " Notice " + sender->getNickname() +
							" :Erreur: le transfert de fichier a echoue");
	}
}

// Bonus Bot
void Server::initBot(){
	if(!_bot){
		_bot = new Bot(this);	// creer le bot
		if(_bot->init()){
			_bot->activate();
			Utils::logMessage("Bot IRC initialise et actif");
		}
		else{
			Utils::logMessage("Erreur lors de l'initialisation du bot IRC", true);
			delete _bot;	// Supprimer le bot en cas d'erreur
			_bot = NULL;
		}
	}
}

void Server::handleBotCommand(Client* client, const std::string& command, const std::string& params){
	if(!_bot){
		client->sendMessage(":" + _serverName + " Notice " + client->getNickname() +
							" :Erreur: le bot IRC n'est pas actif");
		return;
	}
	std::vector<std::string> paramsVec = Utils::split(params, ' ');	// Séparer les paramètres
	_bot->processCommand(client, command, paramsVec);	//commande du bot
}


void Server::setupServerSocket(){
	//creer le socket serveur
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(_serverSocket < 0){
		throw std::runtime_error("Erreur lors de la creation du socket serveur: " + std::string(strerror(errno)));
	}
	int opt = 1;
	if(setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		close(_serverSocket);
		throw std::runtime_error("Erreur lors de la configuration du socket serveur: " + std::string(strerror(errno)));
	}

	int flags = fcntl(_serverSocket, F_GETFL, 0);
	if(flags == -1){
		close(_serverSocket);
		throw std::runtime_error("Erreur lors de la récupération des flags du socket serveur: " + std::string(strerror(errno)));
	}
	if(fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1){
		close(_serverSocket);
		throw std::runtime_error("Erreur lors de la configuration du socket serveur en mode non-bloquant: " + std::string(strerror(errno)));
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));	// Initialiser la structure d'adresse
	serverAddr.sin_family = AF_INET;	// IPv4
	serverAddr.sin_addr.s_addr = INADDR_ANY;	// Accepter toutes les adresses
	serverAddr.sin_port = htons(_port);	// Port d'ecoute
	if(bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){ //lire socket
		close(_serverSocket);
		throw std::runtime_error("Erreur lors de la liaison du socket serveur: " + std::string(strerror(errno)));
	}
	if(listen(_serverSocket, 5) < 0){ //mettre le socket en ecoute
		close(_serverSocket);
		throw std::runtime_error("Erreur lors de l'écoute du socket serveur: " + std::string(strerror(errno)));
	}

	_fds[0].fd = _serverSocket;	// Ajouter le socket serveur au tableau de descripteurs
	_fds[0].events = POLLIN;	// Événement de lecture

	Utils::logMessage("Socket serveur configuré sur le port " + Utils::toString(_port));	// Log de configuration
}

void Server::acceptNewConnection(){
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &addrLen);	// Accepter la nouvelle connexion
	if(clientFd < 0){
		Utils::logMessage("Erreur lors de l'acceptation d'une nouvelle connexion: " + std::string(strerror(errno)), true);
		return;
	}
	int flags = fcntl(clientFd, F_GETFL, 0); //config socket
	if(flags != -1){
		fcntl(clientFd, F_SETFL, flags | O_NONBLOCK); //mode non-bloquant
	}
	if(_nfds >= MAX_CLIENTS + 1){
		close(clientFd);	// Fermer la connexion si le nombre maximum de clients est atteint
		Utils::logMessage("Nombre maximum de clients atteint, connexion refusee", true);
		return;
	}

	Client* client = new Client(clientFd, this);
	char hostStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), hostStr, INET6_ADDRSTRLEN);	// Convertir l'adresse IP en chaîne
	client->setHostname(hostStr);	// Définir le nom d'hôte du client

	_clients[clientFd] = client;	// Ajouter le client à la map des clients
	_fds[_nfds].fd = clientFd;	// Ajouter le descripteur de fichier du client au tableau de descripteurs
	_fds[_nfds].events = POLLIN;	// Événement de lecture
	_nfds++;	// Incrementer le nombre de descripteurs suivis par poll
	Utils::logMessage("Nouvelle connexion accepte: " + client->toString());	// Log de la nouvelle connexion
}

void Server::handleClientMessage(int clientFd){
	std::map<int, Client*>::iterator it = _clients.find(clientFd);	// Rechercher le client par son descripteur de fichier
	if(it == _clients.end())
		return;	// Client non trouvé
	Client* client = it->second;	// Obtenir le client
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));	// Initialiser le buffer
	ssize_t bytesRead = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);	// Lire les données du client
	if(bytesRead <= 0){
		if(bytesRead <= 0 || errno != EWOULDBLOCK){
			removeClient(clientFd);	// Supprimer le client si la lecture échoue
		}
		return;
	}
	client->appendToBuffer(std::string (buffer, bytesRead));	// Ajouter les données lues au buffer du client
	std::string clientBuffer = client->getBuffer();	// Obtenir le buffer du client
	size_t pos;
	while((pos = clientBuffer.find("\r\n")) != std::string::npos ||
		(pos = clientBuffer.find("\n")) != std::string::npos){
			std::string message = clientBuffer.substr(0, pos);	// Extraire le message
			clientBuffer = clientBuffer.substr(pos + (clientBuffer[pos] == '\r' ? 2 : 1));
			if(!message.empty()){
				std::string cmdName;
				size_t spacePos = message.find(' ');	// Trouver le premier espace
				if(spacePos != std::string::npos){
					cmdName = message.substr(0, spacePos);	// Extraire le nom de la commande
				}else {
					cmdName = message;	// Pas d'espace, le message est la commande
				}
				cmdName = Utils::toUpper(cmdName);	// Convertir le nom de la commande en majuscules
				if(client->getStatus() == CONNECTING && cmdName != "PASS" && cmdName != "QUIT" && cmdName != "PING"){
					client->sendMessage("464 : You must provide a valid password first with PASS command");	// Envoyer un message d'erreur");
					continue;	// Sortir de la boucle si le client n'est pas enregistré
				}
				bool isValidCmd = _commandHandler->isCommandValid(cmdName);
				if(isValidCmd){
					Utils::logMessage("Message recu de " + client->getNickname() + ": " + message);	// Log du message reçu
				}
				_commandHandler->executeCommand(client, message);	// Traiter la commande
			// message = Utils::trim(message);	// gestion espaces

			// size_t nextPos = (clientBuffer[pos] == '\r' && pos + 1 < clientBuffer.size() && clientBuffer[pos + 1] == '\n') ? pos + 2 : pos + 1;
			// clientBuffer = clientBuffer.substr(nextPos);	// Mettre à jour le buffer du client
			// if(!message.empty()){
			// 	Utils::logMessage("Message recu de " + client->getNickname() + ": " + message);	// Log du message reçu
			// 	_commandHandler->executeCommand(client, message);	// Traiter la commande
			}
	}
	client->clearBuffer();	// Vider le buffer du client
	client->appendToBuffer(clientBuffer);	// Mettre a jour le buffer du client
	client->processMessages();
}
