#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>       // Pour les chaînes de caractères
# include <vector>       // Pour stocker des collections de données
# include <queue>        // Pour la file de messages
# include <iostream>     // Pour les entrées/sorties standard

# include "Server.hpp"
# include "Channel.hpp"

class Server;
class Channel;

// Énumération des différents états d'un client
enum ClientStatus 
{
    CONNECTING,      // Client se connecte, n'a pas encore envoyé le password
    PASSWORD_SENT,   // Password envoyé, en attente de NICK et USER
    REGISTERED,      // Enregistré (NICK et USER validés)
    DISCONNECTED     // Déconnecté (arrêté ou erreur)
};

class Client 
{
private:
    int             _fd;                // Descripteur de fichier du socket client
    std::string     _nickname;          // Pseudo du client
    std::string     _username;          // Nom d'utilisateur
    std::string     _hostname;          // Nom d'hôte
    std::string     _realname;          // Nom réel
    std::string     _buffer;            // Buffer de réception des messages
    ClientStatus    _status;            // État du client
    Server*         _server;            // Pointeur vers le serveur
    std::vector<Channel*> _channels;    // Canaux auxquels le client est connecté
    std::queue<std::string> _messages;  // File d'attente des messages à envoyer
    std::string     _away_message;      // Message d'absence (bonus)
    bool            _isAway;            // Client absent ou non (bonus)
    bool            _isOperator;        // Client est opérateur global
    std::string     _lastPong;          // Temps du dernier PONG reçu pour le PING
    
public:
    // Constructeur et destructeur
    Client(int fd, Server* server);
    ~Client();
    
    // Getters et setters
    int getFd() const;
    const std::string& getNickname() const;
    void setNickname(const std::string& nickname);
    const std::string& getUsername() const;
    void setUsername(const std::string& username);
    const std::string& getHostname() const;
    void setHostname(const std::string& hostname);
    const std::string& getRealname() const;
    void setRealname(const std::string& realname);
    ClientStatus getStatus() const;
    void setStatus(ClientStatus status);
    bool isOperator() const;
    void setOperator(bool op);
    
    // Gestion du buffer de réception
    void appendToBuffer(const std::string& data);
    std::string getBuffer() const;
    void clearBuffer();
    
    // Gestion des canaux
    void joinChannel(Channel* channel);
    void leaveChannel(Channel* channel);
    bool isInChannel(Channel* channel) const;
    bool isInChannel(const std::string& channelName) const;
    std::vector<Channel*> getChannels() const;
    
    // Communication
    void sendMessage(const std::string& message);
    void sendReply(const std::string& reply);
    void sendNotice(const std::string& notice);
    void processMessages();
    
    // Fonctions d'état
    bool isRegistered() const;
    
    // Gestion du away (bonus)
    void setAway(bool away, const std::string& message = "");
    bool isAway() const;
    const std::string& getAwayMessage() const;
    
    // Conversion en string pour l'affichage
    std::string toString() const;
};

#endif