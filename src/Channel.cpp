#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"

/**
 * Constructeur de la classe Channel
 * arg name Nom du canal
 * arg creator Client créateur du canal
 */
Channel::Channel(const std::string& name, Client* creator)
    : _name(name),                // Initialiser le nom du canal
      _topic(""),                 // Pas de sujet initial
      _modes(0),                  // Pas de modes initiaux
      _password(""),              // Pas de mot de passe initial
      _userLimit(0),              // Pas de limite d'utilisateurs
      _creationTime(time(NULL))   // Timestamp de création
{
    // Ajouter le créateur comme client et opérateur
    addClient(creator, true);

    // Log de création du canal
    Utils::logMessage("Canal créé: " + _name + " par " + creator->getNickname());
}

/**
 * Destructeur de la classe Channel
 */
Channel::~Channel()
{
    // Log de destruction du canal
    Utils::logMessage("Canal détruit: " + _name);

    // Vider la map des clients
    _clients.clear();
}

/**
 * Récupère le nom du canal
 * return Nom du canal
 */
const std::string& Channel::getName() const
{
    return _name;
}

/**
 * Récupère le sujet du canal
 * return Sujet du canal
 */
const std::string& Channel::getTopic() const
{
    return _topic;
}

/**
 * Définit le sujet du canal
 * arg topic Nouveau sujet
 * arg setter Client qui définit le sujet
 */
void Channel::setTopic(const std::string& topic, Client* setter)
{
    // Mettre à jour le sujet
    _topic = topic;

    // Log de changement de sujet
    if (setter)
    {
        Utils::logMessage("Sujet du canal " + _name + " défini par " + setter->getNickname() + ": " + _topic);
    }
    else
    {
        Utils::logMessage("Sujet du canal " + _name + " défini: " + _topic);
    }
}

/**
 * Récupère les modes du canal
 * return Modes du canal
 */
unsigned int Channel::getModes() const
{
    return _modes;
}

/**
 * Récupère le mot de passe du canal
 * return Mot de passe du canal
 */
const std::string& Channel::getPassword() const
{
    return _password;
}

/**
 * Définit le mot de passe du canal
 * arg password Nouveau mot de passe
 */
void Channel::setPassword(const std::string& password)
{
    // Mettre à jour le mot de passe
    _password = password;

    // Log de changement de mot de passe
    Utils::logMessage("Mot de passe du canal " + _name + " défini");
}

/**
 * Récupère la limite d'utilisateurs du canal
 * return Limite d'utilisateurs
 */
unsigned int Channel::getUserLimit() const
{
    return _userLimit;
}

/**
 * Définit la limite d'utilisateurs du canal
 * arg limit Nouvelle limite d'utilisateurs
 */
void Channel::setUserLimit(unsigned int limit)
{
    // Mettre à jour la limite d'utilisateurs
    _userLimit = limit;

    // Log de changement de limite d'utilisateurs
    Utils::logMessage("Limite d'utilisateurs du canal " + _name + " définie à " + Utils::toString(_userLimit));
}

/**
 * Récupère le timestamp de création du canal
 * return Timestamp de création
 */
time_t Channel::getCreationTime() const
{
    return _creationTime;
}

/**
 * Vérifie si le canal a un mode spécifique
 * arg mode Mode à vérifier
 * return true si le canal a ce mode, false sinon
 */
bool Channel::hasMode(ChannelMode mode) const
{
    return (_modes & mode) != 0;
}

/**
 * Définit ou supprime un mode du canal
 * arg mode Mode à définir/supprimer
 * arg enabled true pour définir, false pour supprimer
 */
void Channel::setMode(ChannelMode mode, bool enabled)
{
    // Mettre à jour les modes
    if (enabled)
    {
        _modes |= mode;
    }
    else
    {
        _modes &= ~mode;
    }

    // Log de changement de mode
    std::string modeChar;
    switch (mode)
    {
        case MODE_INVITE_ONLY:
            modeChar = "i";
            break;
        case MODE_TOPIC_LOCKED:
            modeChar = "t";
            break;
        case MODE_PASSWORD:
            modeChar = "k";
            break;
        case MODE_USER_LIMIT:
            modeChar = "l";
            break;
        default:
            modeChar = "?";
            break;
    }

    Utils::logMessage("Mode du canal " + _name + " " + (enabled ? "+" : "-") + modeChar);
}

/**
 * Ajoute un client au canal
 * arg client Client à ajouter
 * arg asOperator true pour ajouter comme opérateur, false sinon
 */
void Channel::addClient(Client* client, bool asOperator)
{
    // Vérifier que le client existe
    if (!client)
    {
        return;
    }

    // Vérifier que le client n'est pas déjà dans le canal
    if (hasClient(client))
    {
        return;
    }

    // Ajouter le client à la map
    _clients[client] = asOperator ? USER_MODE_OPERATOR : 0;

    // Faire rejoindre le canal au client
    client->joinChannel(this);

    // Log d'ajout du client
    Utils::logMessage("Client " + client->getNickname() + " a rejoint le canal " + _name +
                     (asOperator ? " comme opérateur" : ""));
}

/**
 * Supprime un client du canal
 * arg client Client à supprimer
 */
void Channel::removeClient(Client* client)
{
    // Vérifier que le client existe
    if (!client)
    {
        return;
    }

    // Rechercher le client dans la map
    std::map<Client*, unsigned int>::iterator it = _clients.find(client);
    if (it == _clients.end())
    {
        return;
    }

    // Supprimer le client de la map
    _clients.erase(it);

    // Faire quitter le canal au client
    client->leaveChannel(this);

    // Log de suppression du client
    Utils::logMessage("Client " + client->getNickname() + " a quitté le canal " + _name);
}

/**
 * Vérifie si un client est dans le canal
 * arg client Client à vérifier
 * return true si le client est dans le canal, false sinon
 */
bool Channel::hasClient(Client* client) const
{
    // Vérifier que le client existe
    if (!client)
    {
        return false;
    }

    // Rechercher le client dans la map
    return _clients.find(client) != _clients.end();
}

/**
 * Vérifie si un client est dans le canal par son pseudo
 * arg nickname Pseudo du client à vérifier
 * return true si le client est dans le canal, false sinon
 */
bool Channel::hasClient(const std::string& nickname) const
{
    // Parcourir tous les clients
    for (std::map<Client*, unsigned int>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (Utils::toLower(it->first->getNickname()) == Utils::toLower(nickname))
        {
            return true;
        }
    }

    return false;
}

/**
 * Récupère un client dans le canal par son pseudo
 * arg nickname Pseudo du client à récupérer
 * return Pointeur vers le client ou NULL si non trouvé
 */
Client* Channel::getClient(const std::string& nickname) const
{
    // Parcourir tous les clients
    for (std::map<Client*, unsigned int>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (Utils::toLower(it->first->getNickname()) == Utils::toLower(nickname))
        {
            return it->first;
        }
    }

    return NULL;
}

/**
 * Récupère la liste des clients dans le canal
 * return Vecteur de pointeurs vers les clients
 */
std::vector<Client*> Channel::getClients() const
{
    // Créer le vecteur
    std::vector<Client*> clients;

    // Ajouter tous les clients
    for (std::map<Client*, unsigned int>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        clients.push_back(it->first);
    }

    return clients;
}

/**
 * Récupère la liste des opérateurs du canal
 * return Vecteur de pointeurs vers les opérateurs
 */
std::vector<Client*> Channel::getOperators() const
{
    // Créer le vecteur
    std::vector<Client*> operators;

    // Ajouter tous les opérateurs
    for (std::map<Client*, unsigned int>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if ((it->second & USER_MODE_OPERATOR) != 0)
        {
            operators.push_back(it->first);
        }
    }

    return operators;
}

/**
 * Vérifie si un client est opérateur du canal
 * arg client Client à vérifier
 * return true si le client est opérateur, false sinon
 */
bool Channel::isOperator(Client* client) const
{
    // Vérifier que le client existe
    if (!client)
    {
        return false;
    }

    // Rechercher le client dans la map
    std::map<Client*, unsigned int>::const_iterator it = _clients.find(client);
    if (it == _clients.end())
    {
        return false;
    }

    // Vérifier si le client est opérateur
    return (it->second & USER_MODE_OPERATOR) != 0;
}

/**
 * Définit ou supprime le statut d'opérateur d'un client
 * arg client Client à modifier
 * arg op true pour définir comme opérateur, false pour supprimer
 */
void Channel::setOperator(Client* client, bool op)
{
    // Vérifier que le client existe
    if (!client)
    {
        return;
    }

    // Rechercher le client dans la map
    std::map<Client*, unsigned int>::iterator it = _clients.find(client);
    if (it == _clients.end())
    {
        return;
    }

    // Mettre à jour le statut d'opérateur
    if (op)
    {
        it->second |= USER_MODE_OPERATOR;
    }
    else
    {
        it->second &= ~USER_MODE_OPERATOR;
    }

    // Log de changement de statut d'opérateur
    Utils::logMessage("Client " + client->getNickname() + " est " + (op ? "maintenant" : "plus un") +
                     " opérateur du canal " + _name);
}

/**
 * Vérifie si un client a le droit de parole
 * arg client Client à vérifier
 * return true si le client a le droit de parole, false sinon
 */
bool Channel::hasVoice(Client* client) const
{
    // Vérifier que le client existe
    if (!client)
    {
        return false;
    }

    // Rechercher le client dans la map
    std::map<Client*, unsigned int>::const_iterator it = _clients.find(client);
    if (it == _clients.end())
    {
        return false;
    }

    // Vérifier si le client a le droit de parole
    return (it->second & USER_MODE_VOICE) != 0;
}

/**
 * Définit ou supprime le droit de parole d'un client
 * arg client Client à modifier
 * arg voice true pour définir le droit de parole, false pour supprimer
 */
void Channel::setVoice(Client* client, bool voice)
{
    // Vérifier que le client existe
    if (!client)
    {
        return;
    }

    // Rechercher le client dans la map
    std::map<Client*, unsigned int>::iterator it = _clients.find(client);
    if (it == _clients.end())
    {
        return;
    }

    // Mettre à jour le droit de parole
    if (voice)
    {
        it->second |= USER_MODE_VOICE;
    }
    else
    {
        it->second &= ~USER_MODE_VOICE;
    }

    // Log de changement de droit de parole
    Utils::logMessage("Client " + client->getNickname() + " a " + (voice ? "maintenant" : "perdu") +
                     " le droit de parole dans le canal " + _name);
}

/**
 * Récupère le nombre de clients dans le canal
 * return Nombre de clients
 */
unsigned int Channel::getClientCount() const
{
    return _clients.size();
}

/**
 * Ajoute un utilisateur à la liste des invités
 * arg nickname Pseudo de l'utilisateur à inviter
 */
void Channel::inviteUser(const std::string& nickname)
{
    // Ajouter l'utilisateur à la liste des invités
    _invitedUsers.insert(Utils::toLower(nickname));

    // Log d'invitation
    Utils::logMessage("Utilisateur " + nickname + " invité au canal " + _name);
}

/**
 * Vérifie si un utilisateur est invité
 * arg nickname Pseudo de l'utilisateur à vérifier
 * return true si l'utilisateur est invité, false sinon
 */
bool Channel::isInvited(const std::string& nickname) const
{
    return _invitedUsers.find(Utils::toLower(nickname)) != _invitedUsers.end();
}

/**
 * Supprime un utilisateur de la liste des invités
 * arg nickname Pseudo de l'utilisateur à supprimer
 */
void Channel::removeInvite(const std::string& nickname)
{
    // Supprimer l'utilisateur de la liste des invités
    _invitedUsers.erase(Utils::toLower(nickname));
}

/**
 * Diffuse un message à tous les clients du canal
 * arg message Message à diffuser
 * arg exclude Client à exclure (optionnel)
 */
void Channel::broadcast(const std::string& message, Client* exclude)
{
    // Parcourir tous les clients
    for (std::map<Client*, unsigned int>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        // Vérifier si le client doit être exclu
        if (it->first != exclude) {
            // Envoyer le message
            it->first->sendMessage(message);
        }
    }
}

/**
 * Vérifie si un client peut rejoindre le canal
 * arg client Client à vérifier
 * arg password Mot de passe fourni (optionnel)
 * return true si le client peut rejoindre le canal, false sinon
 */
bool Channel::clientCanJoin(Client* client, const std::string& password) const
{
    // Vérifier que le client existe
    if (!client)
    {
        return false;
    }

    // Vérifier si le canal est sur invitation uniquement
    if (hasMode(MODE_INVITE_ONLY) && !isInvited(client->getNickname()))
    {
        return false;
    }

    // Vérifier si le canal est protégé par mot de passe
    if (hasMode(MODE_PASSWORD) && password != _password)
    {
        return false;
    }

    // Vérifier si le canal a atteint sa limite d'utilisateurs
    if (hasMode(MODE_USER_LIMIT) && getClientCount() >= _userLimit)
    {
        return false;
    }

    return true;
}

/**
 * Vérifie si un client peut changer le sujet du canal
 * arg client Client à vérifier
 * return true si le client peut changer le sujet, false sinon
 */
bool Channel::clientCanChangeTopic(Client* client) const
{
    // Vérifier que le client existe
    if (!client)
    {
        return false;
    }

    // Vérifier si le client est dans le canal
    if (!hasClient(client))
    {
        return false;
    }

    // Vérifier si le sujet est verrouillé
    if (hasMode(MODE_TOPIC_LOCKED))
    {
        // Si le sujet est verrouillé, seuls les opérateurs peuvent le changer
        return isOperator(client);
    }

    // Si le sujet n'est pas verrouillé, tous les clients peuvent le changer
    return true;
}

/**
 * Convertit les informations du canal en chaîne de caractères
 * return Chaîne de caractères représentant le canal
 */
std::string Channel::toString() const
{
    // Créer la chaîne de caractères
    std::stringstream ss;
    ss << _name << " (" << getClientCount() << " clients)";

    // Ajouter les modes
    std::string modes;
    if (hasMode(MODE_INVITE_ONLY)) modes += "i";
    if (hasMode(MODE_TOPIC_LOCKED)) modes += "t";
    if (hasMode(MODE_PASSWORD)) modes += "k";
    if (hasMode(MODE_USER_LIMIT)) modes += "l";

    if (!modes.empty())
    {
        ss << " [+" << modes << "]";
    }

    // Ajouter le sujet
    if (!_topic.empty())
    {
        ss << " Topic: " << _topic;
    }

    return ss.str();
}