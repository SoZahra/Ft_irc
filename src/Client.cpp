#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Utils.hpp"
#include <unistd.h>  // Pour close() et autres fonctions POSIX
#include <cstring>   // Pour strerror

/**
 * Constructeur de la classe Client
 * arg fd Descripteur de fichier du socket client
 * arg server Pointeur vers le serveur IRC
 */
Client::Client(int fd, Server* server)
    : _fd(fd),                    // Initialiser le descripteur de fichier
      _nickname(""),              // Pseudo vide initialement
      _username(""),              // Nom d'utilisateur vide initialement
      _hostname(""),              // Nom d'hôte vide initialement
      _realname(""),              // Nom réel vide initialement
      _buffer(""),                // Buffer de réception vide initialement
      _status(CONNECTING),        // État initial: se connecte
      _server(server),            // Pointeur vers le serveur
      _isAway(false),             // Client n'est pas absent initialement
      _isOperator(false),         // Client n'est pas opérateur initialement
      _lastPong("")               // Pas de PONG reçu initialement
{
    // Log de création du client
    Utils::logMessage("Client créé avec fd " + Utils::toString(_fd));
}

/**
 * Destructeur de la classe Client
 */
Client::~Client()
{
    // Log de destruction du client
    Utils::logMessage("Client détruit: " + toString());
}

/**
 * Récupère le descripteur de fichier du client
 * return Descripteur de fichier
 */
int Client::getFd() const
{
    return _fd;
}

/**
 * Récupère le pseudo du client
 * return Pseudo du client
 */
const std::string& Client::getNickname() const
{
    return _nickname;
}

/**
 * Définit le pseudo du client
 * arg nickname Nouveau pseudo
 */
void Client::setNickname(const std::string& nickname)
{
    // Enregistrer l'ancien pseudo pour le log
    std::string oldNickname = _nickname;

    // Mettre à jour le pseudo
    _nickname = nickname;

    // Log de changement de pseudo
    if (!oldNickname.empty())
    {
        Utils::logMessage("Client " + oldNickname + " a changé son pseudo en " + _nickname);
    }
    else
    {
        Utils::logMessage("Client a défini son pseudo: " + _nickname);
    }
}

/**
 * Récupère le nom d'utilisateur du client
 * return Nom d'utilisateur
 */
const std::string& Client::getUsername() const
{
    return _username;
}

/**
 * Définit le nom d'utilisateur du client
 * arg username Nouveau nom d'utilisateur
 */
void Client::setUsername(const std::string& username)
{
    // Mettre à jour le nom d'utilisateur
    _username = username;

    // Log de définition du nom d'utilisateur
    Utils::logMessage("Client " + _nickname + " a défini son nom d'utilisateur: " + _username);
}

/**
 * Récupère le nom d'hôte du client
 * return Nom d'hôte
 */
const std::string& Client::getHostname() const
{
    return _hostname;
}

/**
 * Définit le nom d'hôte du client
 * arg hostname Nouveau nom d'hôte
 */
void Client::setHostname(const std::string& hostname)
{
    // Mettre à jour le nom d'hôte
    _hostname = hostname;
}

/**
 * Récupère le nom réel du client
 * return Nom réel
 */
const std::string& Client::getRealname() const
{
    return _realname;
}

/**
 * Définit le nom réel du client
 * arg realname Nouveau nom réel
 */
void Client::setRealname(const std::string& realname)
{
    // Mettre à jour le nom réel
    _realname = realname;

    // Log de définition du nom réel
    Utils::logMessage("Client " + _nickname + " a défini son nom réel: " + _realname);
}

/**
 * Récupère l'état du client
 * return État du client
 */
ClientStatus Client::getStatus() const
{
    return _status;
}

/**
 * Définit l'état du client
 * arg status Nouvel état
 */
void Client::setStatus(ClientStatus status)
{
    // Mettre à jour l'état
    _status = status;

    // Log de changement d'état
    std::string statusStr;
    switch (status) {
        case CONNECTING:
            statusStr = "CONNECTING";
            break;
        case PASSWORD_SENT:
            statusStr = "PASSWORD_SENT";
            break;
        case REGISTERED:
            statusStr = "REGISTERED";
            break;
        case DISCONNECTED:
            statusStr = "DISCONNECTED";
            break;
        default:
            statusStr = "UNKNOWN";
            break;
    }

    Utils::logMessage("Client " + _nickname + " a changé d'état: " + statusStr);
}

/**
 * Vérifie si le client est un opérateur
 * return true si le client est un opérateur, false sinon
 */
bool Client::isOperator() const
{
    return _isOperator;
}

/**
 * Définit si le client est un opérateur
 * arg op true pour définir comme opérateur, false sinon
 */
void Client::setOperator(bool op)
{
    // Mettre à jour le statut d'opérateur
    _isOperator = op;

    // Log de changement de statut d'opérateur
    if (op)
    {
        Utils::logMessage("Client " + _nickname + " est maintenant un opérateur");
    }
    else
    {
        Utils::logMessage("Client " + _nickname + " n'est plus un opérateur");
    }
}

/**
 * Ajoute des données au buffer de réception
 * arg data Données à ajouter
 */
void Client::appendToBuffer(const std::string& data)
{
    // Ajouter les données au buffer
    _buffer += data;
}

/**
 * Récupère le buffer de réception
 * return Buffer de réception
 */
std::string Client::getBuffer() const
{
    return _buffer;
}

/**
 * Vide le buffer de réception
 */
void Client::clearBuffer()
{
    _buffer.clear();
}

/**
 * Fait rejoindre un canal au client
 * arg channel Pointeur vers le canal à rejoindre
 */
void Client::joinChannel(Channel* channel)
{
    // Vérifier que le canal existe
    if (!channel)
    {
        return;
    }

    // Vérifier que le client n'est pas déjà dans le canal
    if (isInChannel(channel))
    {
        return;
    }

    // Ajouter le canal à la liste des canaux du client
    _channels.push_back(channel);

    // Log de rejointe du canal
    Utils::logMessage("Client " + _nickname + " a rejoint le canal: " + channel->getName());
}

/**
 * Fait quitter un canal au client
 * arg channel Pointeur vers le canal à quitter
 */
void Client::leaveChannel(Channel* channel)
{
    // Vérifier que le canal existe
    if (!channel)
    {
        return;
    }

    // Rechercher le canal dans la liste des canaux du client
    for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (*it == channel)
        {
            // Supprimer le canal de la liste
            _channels.erase(it);

            // Log de départ du canal
            Utils::logMessage("Client " + _nickname + " a quitté le canal: " + channel->getName());

            return;
        }
    }
}

/**
 * Vérifie si le client est dans un canal
 * arg channel Pointeur vers le canal à vérifier
 * return true si le client est dans le canal, false sinon
 */
bool Client::isInChannel(Channel* channel) const
{
    // Vérifier que le canal existe
    if (!channel)
    {
        return false;
    }

    // Rechercher le canal dans la liste des canaux du client
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i] == channel)
        {
            return true;
        }
    }

    return false;
}

/**
 * Vérifie si le client est dans un canal par son nom
 * arg channelName Nom du canal à vérifier
 * return true si le client est dans le canal, false sinon
 */
bool Client::isInChannel(const std::string& channelName) const
{
    // Rechercher le canal dans la liste des canaux du client
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (Utils::toLower(_channels[i]->getName()) == Utils::toLower(channelName))
        {
            return true;
        }
    }

    return false;
}

/**
 * Récupère la liste des canaux auxquels le client est connecté
 * return Vecteur de pointeurs vers les canaux
 */
std::vector<Channel*> Client::getChannels() const
{
    return _channels;
}

/**
 * Envoie un message au client
 * arg message Message à envoyer
 */
void Client::sendMessage(const std::string& message)
{
    // Ajouter le message à la file d'attente
    _messages.push(message + "\r\n");

    // Traiter les messages immédiatement
    processMessages();
}

/**
 * Envoie une réponse IRC au client
 * arg reply Réponse à envoyer
 */
void Client::sendReply(const std::string& reply)
{
    // Envoyer la réponse sous forme de message
    sendMessage(":" + _server->getServerName() + " " + reply);
}

/**
 * Envoie une notification au client
 * arg notice Notification à envoyer
 */
void Client::sendNotice(const std::string& notice)
{
    // Envoyer la notification sous forme de message
    sendMessage(":" + _server->getServerName() + " NOTICE " + _nickname + " :" + notice);
}

/**
 * Traite les messages en attente
 */
void Client::processMessages()
{
    // Vérifier s'il y a des messages à envoyer
    while (!_messages.empty())
    {
        // Récupérer le premier message
        std::string message = _messages.front();

        // Envoyer le message
        ssize_t bytesSent = send(_fd, message.c_str(), message.length(), MSG_NOSIGNAL);

        // Vérifier les erreurs
        if (bytesSent < 0)
        {
            // Erreur, vérifier si temporaire
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Temporaire, attendre
                break;
            }
            else
            {
                // Erreur permanente, déconnecter le client
                Utils::logMessage("Erreur lors de l'envoi d'un message: " + std::string(strerror(errno)), true);
                setStatus(DISCONNECTED);
                break;
            }
        } else if (bytesSent < static_cast<ssize_t>(message.length()))
        {
            // Message partiellement envoyé, stocker le reste
            _messages.front() = message.substr(bytesSent);
            break;
        }
        else
        {
            // Message complètement envoyé, le supprimer de la file
            _messages.pop();
        }
    }
}

/**
 * Vérifie si le client est enregistré
 * return true si le client est enregistré, false sinon
 */
bool Client::isRegistered() const
{
    return _status == REGISTERED;
}

/**
 * Définit si le client est absent
 * arg away true pour marquer comme absent, false sinon
 * arg message Message d'absence (optionnel)
 */
void Client::setAway(bool away, const std::string& message)
{
    // Mettre à jour le statut d'absence
    _isAway = away;

    // Mettre à jour le message d'absence
    if (away)
    {
        _away_message = message;
        Utils::logMessage("Client " + _nickname + " est maintenant absent: " + message);
    }
    else
    {
        _away_message.clear();
        Utils::logMessage("Client " + _nickname + " n'est plus absent");
    }
}

/**
 * Vérifie si le client est absent
 * return true si le client est absent, false sinon
 */
bool Client::isAway() const
{
    return _isAway;
}

/**
 * Récupère le message d'absence du client
 * return Message d'absence
 */
const std::string& Client::getAwayMessage() const
{
    return _away_message;
}

/**
 * Convertit les informations du client en chaîne de caractères
 * return Chaîne de caractères représentant le client
 */
std::string Client::toString() const
{
    // Créer la chaîne de caractères
    std::stringstream ss;
    ss << _nickname << "!" << _username << "@" << _hostname;

    // Ajouter l'état
    ss << " [";
    switch (_status)
    {
        case CONNECTING:
            ss << "CONNECTING";
            break;
        case PASSWORD_SENT:
            ss << "PASSWORD_SENT";
            break;
        case REGISTERED:
            ss << "REGISTERED";
            break;
        case DISCONNECTED:
            ss << "DISCONNECTED";
            break;
        default:
            ss << "UNKNOWN";
            break;
    }
    ss << "]";

    // Ajouter les informations supplémentaires
    if (_isOperator)
    {
        ss << " [OPER]";
    }
    if (_isAway)
    {
        ss << " [AWAY]";
    }

    return ss.str();
}