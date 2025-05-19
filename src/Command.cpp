#include "../includes/Command.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/CommandHandler.hpp"
#include "../includes/Utils.hpp"

/**
 * Constructeur de la classe de base Command
 * arg server Pointeur vers le serveur
 * arg name Nom de la commande
 * arg requiresRegistration Si la commande nécessite que le client soit enregistré
 * arg minParams Nombre minimum de paramètres requis
 */
Command::Command(Server* server, const std::string& name, bool requiresRegistration, unsigned int minParams)
    : _server(server),               // Initialiser le pointeur vers le serveur
      _name(name),                   // Initialiser le nom de la commande
      _requiresRegistration(requiresRegistration), // Initialiser si la commande nécessite que le client soit enregistré
      _minParams(minParams)          // Initialiser le nombre minimum de paramètres
{
    // vide
}

/**
 * Destructeur de la classe de base Command
 */
Command::~Command()
{
    // vide
}

/**
 * Récupère le nom de la commande
 * return Nom de la commande
 */
const std::string& Command::getName() const
{
    return _name;
}

/**
 * Vérifie si la commande nécessite que le client soit enregistré
 * return true si la commande nécessite que le client soit enregistré, false sinon
 */
bool Command::requiresRegistration() const
{
    return _requiresRegistration;
}

/**
 * Récupère le nombre minimum de paramètres requis
 * return Nombre minimum de paramètres
 */
unsigned int Command::getMinParams() const
{
    return _minParams;
}

// Implémentation de la commande PASS

/**
 * Constructeur de la commande PASS
 * arg server Pointeur vers le serveur
 */
PassCommand::PassCommand(Server* server)
    : Command(server, "PASS", false, 1) // PASS nécessite 1 paramètre et ne nécessite pas que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande PASS
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void PassCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Vérifier que le client n'est pas déjà enregistré
    if (client->getStatus() != CONNECTING)
    {
        // Client déjà authentifié
        client->sendReply("462 :You may not reregister");
        return;
    }

    // Vérifier si le mot de passe est correct
    if (params[0] != _server->getPassword())
    {
        // Mot de passe incorrect
        client->sendReply("464 :Password incorrect");
        return;
    }

    // Mettre à jour l'état du client
    client->setStatus(PASSWORD_SENT);

    // Log de succès
    Utils::logMessage("Client " + Utils::toString(client->getFd()) + " a envoyé le mot de passe correct");
}

// Implémentation de la commande NICK

/**
 * Constructeur de la commande NICK
 * arg server Pointeur vers le serveur
 */
NickCommand::NickCommand(Server* server)
    : Command(server, "NICK", false, 1) // NICK nécessite 1 paramètre et ne nécessite pas que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande NICK
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void NickCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer le nouveau pseudo
    std::string newNick = params[0];

    // Vérifier si le pseudo est valide
    if (!Utils::isValidNickname(newNick))
    {
        // Pseudo invalide
        client->sendReply("432 " + newNick + " :Erroneous nickname");
        return;
    }

    // Vérifier si le pseudo est déjà utilisé
    if (_server->getClientByNickname(newNick) != NULL)
    {
        // Pseudo déjà utilisé
        client->sendReply("433 " + newNick + " :Nickname is already in use");
        return;
    }

    // Récupérer l'ancien pseudo
    std::string oldNick = client->getNickname();

    // Définir le nouveau pseudo
    client->setNickname(newNick);

    // Envoyer un message NICK si le client est déjà enregistré
    if (client->isRegistered())
    {
        std::string message = ":" + oldNick + "!" + client->getUsername() + "@" + client->getHostname() + " NICK :" + newNick;

        // Envoyer le message au client
        client->sendMessage(message);

        // Diffuser le message à tous les canaux auxquels le client est connecté
        std::vector<Channel*> channels = client->getChannels();
        for (size_t i = 0; i < channels.size(); ++i)
        {
            channels[i]->broadcast(message, client);
        }
    }
    else if (client->getStatus() == PASSWORD_SENT && !client->getUsername().empty())
    {
        // Si le client a envoyé le mot de passe et son nom d'utilisateur, il est maintenant enregistré
        client->setStatus(REGISTERED);

        // Envoyer les messages de bienvenue
        this->sendWelcomeMessages(client);
    }

    // Log de changement de pseudo
    Utils::logMessage("Client " + Utils::toString(client->getFd()) + " a changé son pseudo en " + newNick);
}

/**
 * Envoie les messages de bienvenue à un client nouvellement enregistré
 * arg client Client à qui envoyer les messages
 */
void NickCommand::sendWelcomeMessages(Client* client)
{
    // Message de bienvenue
    client->sendReply("001 :Welcome to the Internet Relay Network " + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname());

    // Informations sur le serveur
    client->sendReply("002 :Your host is " + _server->getServerName() + ", running version ft_irc 1.0");

    // Date de création du serveur
    client->sendReply("003 :This server was created " + _server->getCreationDate());

    // Informations sur le serveur
    client->sendReply("004 " + _server->getServerName() + " ft_irc 1.0 o o");

    // Informations sur les utilisateurs
    client->sendReply("251 :There are " + Utils::toString(_server->getClientCount()) + " users and 0 invisible on 1 servers");

    // Informations sur les opérateurs
    client->sendReply("252 0 :operator(s) online");

    // Informations sur les canaux
    client->sendReply("254 " + Utils::toString(_server->getChannelCount()) + " :channels formed");

    // Informations sur les clients
    client->sendReply("255 :I have " + Utils::toString(_server->getClientCount()) + " clients and 1 servers");

    // Message du jour
    client->sendReply("375 :- " + _server->getServerName() + " Message of the day - ");
    client->sendReply("372 :- Welcome to ft_irc!");
    client->sendReply("372 :- This server is running ft_irc 1.0");
    client->sendReply("372 :- Have fun!");
    client->sendReply("376 :End of /MOTD command");

    // Log d'enregistrement
    Utils::logMessage("Client " + client->getNickname() + " est maintenant enregistré");
}

// Implémentation de la commande USER

/**
 * Constructeur de la commande USER
 * arg server Pointeur vers le serveur
 */
UserCommand::UserCommand(Server* server)
    : Command(server, "USER", false, 4) // USER nécessite 4 paramètres et ne nécessite pas que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande USER
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void UserCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Vérifier que le client n'est pas déjà enregistré
    if (client->isRegistered())
    {
        // Client déjà enregistré
        client->sendReply("462 :You may not reregister");
        return;
    }

    // Récupérer les paramètres
    std::string username = params[0];
    // params[1] et params[2] sont ignorés
    std::string realname = params[3];

    // Définir le nom d'utilisateur et le nom réel
    client->setUsername(username);
    client->setRealname(realname);

    // Si le client a envoyé le mot de passe et son pseudo, il est maintenant enregistré
    if (client->getStatus() == PASSWORD_SENT && !client->getNickname().empty())
    {
        client->setStatus(REGISTERED);

        // Envoyer les messages de bienvenue
        NickCommand nickCmd(_server);
        nickCmd.sendWelcomeMessages(client);
    }

    // Log d'enregistrement
    Utils::logMessage("Client " + Utils::toString(client->getFd()) + " a défini son nom d'utilisateur à " + username + " et son nom réel à " + realname);
}

// Implémentation de la commande QUIT

/**
 * Constructeur de la commande QUIT
 * arg server Pointeur vers le serveur
 */
QuitCommand::QuitCommand(Server* server)
    : Command(server, "QUIT", false, 0) // QUIT ne nécessite pas de paramètres et ne nécessite pas que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande QUIT
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void QuitCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer le message de départ
    std::string quitMessage = params.size() > 0 ? params[0] : "Quit";

    // Marquer le client comme déconnecté
    client->setStatus(DISCONNECTED);

    // Envoyer un message QUIT à tous les canaux auxquels le client est connecté
    std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " QUIT :" + quitMessage;
    std::vector<Channel*> channels = client->getChannels();

    for (size_t i = 0; i < channels.size(); ++i)
    {
        channels[i]->broadcast(message, client);
    }

    // Log de déconnexion
    Utils::logMessage("Client " + client->getNickname() + " s'est déconnecté: " + quitMessage);

    // Le client sera supprimé par le serveur lors de la prochaine vérification
}

// Implémentation de la commande JOIN

/**
 * Constructeur de la commande JOIN
 * arg server Pointeur vers le serveur
 */
JoinCommand::JoinCommand(Server* server)
    : Command(server, "JOIN", true, 1) // JOIN nécessite au moins 1 paramètre et nécessite que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande JOIN
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void JoinCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer le nom du canal
    std::string channelName = params[0];

    // Récupérer le mot de passe du canal (optionnel)
    std::string password = params.size() > 1 ? params[1] : "";

    // Vérifier si le nom du canal est valide
    if (!Utils::isValidChannelName(channelName))
    {
        // Nom de canal invalide
        client->sendReply("403 " + channelName + " :No such channel");
        return;
    }

    // Rechercher le canal
    Channel* channel = _server->getChannel(channelName);

    // Si le canal n'existe pas, le créer
    if (channel == NULL)
    {
        channel = _server->createChannel(channelName, client);

        // Log de création de canal
        Utils::logMessage("Canal " + channelName + " créé par " + client->getNickname());
    }
    else
    {
        // Vérifier si le client peut rejoindre le canal
        if (!channel->clientCanJoin(client, password))
        {
            // Le client ne peut pas rejoindre le canal
            if (channel->hasMode(MODE_INVITE_ONLY) && !channel->isInvited(client->getNickname()))
            {
                // Canal sur invitation uniquement
                client->sendReply("473 " + channelName + " :Cannot join channel (+i)");
            }
            else if (channel->hasMode(MODE_PASSWORD) && password != channel->getPassword())
            {
                // Mot de passe incorrect
                client->sendReply("475 " + channelName + " :Cannot join channel (+k)");
            }
            else if (channel->hasMode(MODE_USER_LIMIT) && channel->getClientCount() >= channel->getUserLimit())
            {
                // Canal plein
                client->sendReply("471 " + channelName + " :Cannot join channel (+l)");
            }
            else
            {
                // Autre raison
                client->sendReply("474 " + channelName + " :Cannot join channel");
            }
            return;
        }

        // Ajouter le client au canal
        channel->addClient(client, false);
    }

    // Supprimer l'invitation si le client était invité
    if (channel->isInvited(client->getNickname()))
    {
        channel->removeInvite(client->getNickname());
    }

    // Envoyer un message JOIN à tous les clients du canal
    std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " JOIN :" + channelName;
    channel->broadcast(message, NULL);

    // Envoyer le sujet du canal s'il existe
    if (!channel->getTopic().empty())
    {
        client->sendReply("332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
    }

    // Envoyer la liste des membres du canal
    this->sendNames(client, channel);

    // Log de rejointe de canal
    Utils::logMessage("Client " + client->getNickname() + " a rejoint le canal " + channelName);
}

/**
 * Envoie la liste des membres d'un canal à un client
 * arg client Client à qui envoyer la liste
 * arg channel Canal dont on envoie la liste des membres
 */
void JoinCommand::sendNames(Client* client, Channel* channel)
{
    // Récupérer la liste des clients du canal
    std::vector<Client*> channelClients = channel->getClients();

    // Construire la liste des membres
    std::string names;
    for (size_t i = 0; i < channelClients.size(); ++i)
    {
        // Ajouter @ devant le pseudo si le client est opérateur
        if (channel->isOperator(channelClients[i]))
        {
            names += "@";
        }
        else if (channel->hasVoice(channelClients[i]))
        {
            names += "+";
        }

        // Ajouter le pseudo
        names += channelClients[i]->getNickname();

        // Ajouter un espace sauf pour le dernier client
        if (i < channelClients.size() - 1)
        {
            names += " ";
        }
    }

    // Envoyer la liste des membres
    client->sendReply("353 " + client->getNickname() + " = " + channel->getName() + " :" + names);

    // Envoyer la fin de la liste
    client->sendReply("366 " + client->getNickname() + " " + channel->getName() + " :End of /NAMES list");
}

// Implémentation de la commande PART

/**
 * Constructeur de la commande PART
 * arg server Pointeur vers le serveur
 */
PartCommand::PartCommand(Server* server)
    : Command(server, "PART", true, 1) // PART nécessite au moins 1 paramètre et nécessite que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande PART
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void PartCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer le nom du canal
    std::string channelName = params[0];

    // Récupérer le message de départ (optionnel)
    std::string partMessage = params.size() > 1 ? params[1] : "Leaving";
	if(!partMessage.empty() && partMessage[0] == ':')
		partMessage = partMessage.substr(1); // Enlever le ':' au début du message

    // Rechercher le canal
    Channel* channel = _server->getChannel(channelName);
    // Vérifier si le canal existe
    if (channel == NULL)
    {
        // Canal inexistant
        client->sendReply("403 " + channelName + " :No such channel");
        return;
    }
    // Vérifier si le client est dans le canal
    if (!channel->hasClient(client))
    {
        // Client pas dans le canal
        client->sendReply("442 " + channelName + " :You're not on that channel");
        return;
    }

    // Envoyer un message PART à tous les clients du canal
    std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " PART " + channelName + " :" + partMessage;
    channel->broadcast(message, NULL);

    // Supprimer le client du canal
    channel->removeClient(client);

    // Supprimer le canal s'il est vide
    if (channel->getClientCount() == 0)
    {
        _server->removeChannel(channelName);
    }

    // Log de départ de canal
    Utils::logMessage("Client " + client->getNickname() + " a quitté le canal " + channelName);
}

// Implémentation de la commande PRIVMSG

/**
 * Constructeur de la commande PRIVMSG
 * arg server Pointeur vers le serveur
 */
PrivmsgCommand::PrivmsgCommand(Server* server)
    : Command(server, "PRIVMSG", true, 2) // PRIVMSG nécessite 2 paramètres et nécessite que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande PRIVMSG
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void PrivmsgCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer la cible du message
    std::string target = params[0];

    // Récupérer le message
    std::string message = params[1];

    // Vérifier si la cible commence par #, c'est un canal
    if (target[0] == '#' || target[0] == '&')
    {
        // Message à un canal
        Channel* channel = _server->getChannel(target);

        // Vérifier si le canal existe
        if (channel == NULL)
        {
            // Canal inexistant
            client->sendReply("403 " + target + " :No such channel");
            return;
        }

        // Vérifier si le client est dans le canal
        if (!channel->hasClient(client))
        {
            // Client pas dans le canal
            client->sendReply("442 " + target + " :You're not on that channel");
            return;
        }

        // Envoyer le message à tous les clients du canal sauf l'émetteur
        std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " PRIVMSG " + target + " :" + message;
        channel->broadcast(msg, client);

        // Log de message à un canal
        Utils::logMessage("Client " + client->getNickname() + " a envoyé un message au canal " + target + ": " + message);
    }
    else
    {
        // Message à un utilisateur
        Client* targetClient = _server->getClientByNickname(target);

        // Vérifier si l'utilisateur existe
        if (targetClient == NULL)
        {
            // Utilisateur inexistant
            client->sendReply("401 " + target + " :No such nick/channel");
            return;
        }

        // Envoyer le message à l'utilisateur
        std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " PRIVMSG " + target + " :" + message;
        targetClient->sendMessage(msg);

        // Envoyer une réponse automatique si l'utilisateur est marqué comme absent
        if (targetClient->isAway())
        {
            client->sendReply("301 " + target + " :" + targetClient->getAwayMessage());
        }

        // Log de message à un utilisateur
        Utils::logMessage("Client " + client->getNickname() + " a envoyé un message à " + target + ": " + message);
    }
}

// Implémentation de la commande NOTICE

/**
 * Constructeur de la commande NOTICE
 * arg server Pointeur vers le serveur
 */
NoticeCommand::NoticeCommand(Server* server)
    : Command(server, "NOTICE", true, 2) // NOTICE nécessite 2 paramètres et nécessite que le client soit enregistré
{
    // vide
}

/**
 * Exécute la commande NOTICE
 * arg client Client qui exécute la commande
 * arg params Paramètres de la commande
 */
void NoticeCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer la cible de la notification
    std::string target = params[0];

    // Récupérer le message
    std::string message = params[1];

    // Vérifier si la cible commence par #, c'est un canal
    if (target[0] == '#' || target[0] == '&')
    {
        // Notification à un canal
        Channel* channel = _server->getChannel(target);

        // Vérifier si le canal existe
        if (channel == NULL)
        {
            // Canal inexistant, pas d'erreur pour NOTICE
            return;
        }

        // Vérifier si le client est dans le canal
        if (!channel->hasClient(client))
        {
            // Client pas dans le canal, pas d'erreur pour NOTICE
            return;
        }

        // Envoyer la notification à tous les clients du canal sauf l'émetteur
        std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " NOTICE " + target + " :" + message;
        channel->broadcast(msg, client);

        // Log de notification à un canal
        Utils::logMessage("Client " + client->getNickname() + " a envoyé une notification au canal " + target + ": " + message);
    }
    else
    {
        // Notification à un utilisateur
        Client* targetClient = _server->getClientByNickname(target);

        // Vérifier si l'utilisateur existe
        if (targetClient == NULL)
        {
            // Utilisateur inexistant, pas d'erreur pour NOTICE
            return;
        }

        // Envoyer la notification à l'utilisateur
        std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " NOTICE " + target + " :" + message;
        targetClient->sendMessage(msg);

        // Log de notification à un utilisateur
        Utils::logMessage("Client " + client->getNickname() + " a envoyé une notification à " + target + ": " + message);
    }
}

// Implémentation de la commande MODE
ModeCommand::ModeCommand(Server* server)
    : Command(server, "MODE", true, 1)
{
    // vide
}

void ModeCommand::execute(Client* client, const std::vector<std::string>& params)
{
    // Récupérer le nom du canal ou le pseudo
    std::string target = params[0];

    // Vérifier si la cible commence par #, c'est un canal
    if (target[0] == '#' || target[0] == '&')
    {
        // Mode de canal
        handleChannelMode(client, target, params.size() > 1 ? params[1] : "",
                          std::vector<std::string>(params.begin() + 2, params.end()));
    }
    else
    {
        // Mode d'utilisateur
        handleUserMode(client, target, params.size() > 1 ? params[1] : "");
    }
}
void ModeCommand::handleChannelMode(Client* client, const std::string& channelName,
	const std::string& modes, const std::vector<std::string>& modeParams)
{
// Rechercher le canal
Channel* channel = _server->getChannel(channelName);

// Vérifier si le canal existe
if (channel == NULL)
{
// Canal inexistant
client->sendReply("403 " + channelName + " :No such channel");
return;
}

// Si pas de modes, afficher les modes actuels
if (modes.empty())
{
// Construire la chaîne des modes
std::string modeStr = "+";
if (channel->hasMode(MODE_INVITE_ONLY)) modeStr += "i";
if (channel->hasMode(MODE_TOPIC_LOCKED)) modeStr += "t";
if (channel->hasMode(MODE_PASSWORD)) modeStr += "k";
if (channel->hasMode(MODE_USER_LIMIT)) modeStr += "l";

// Construire la chaîne des paramètres
std::string paramStr;
if (channel->hasMode(MODE_PASSWORD)) paramStr += " " + channel->getPassword();
if (channel->hasMode(MODE_USER_LIMIT)) paramStr += " " + Utils::toString(channel->getUserLimit());

// Envoyer les modes
client->sendReply("324 " + channelName + " " + modeStr + paramStr);
return;
}

// Vérifier si le client est opérateur du canal
if (!channel->isOperator(client))
{
// Client pas opérateur
client->sendReply("482 " + channelName + " :You're not channel operator");
return;
}

// Analyser les modes
bool add = true;  // Par défaut, on ajoute des modes
size_t paramIndex = 0;
std::string modeChanges = "+";  // Pour le message MODE
std::string paramChanges;       // Pour le message MODE

for (size_t i = 0; i < modes.size(); ++i)
{
char mode = modes[i];

if (mode == '+')
{
// Ajouter des modes
add = true;
modeChanges += "+";
} else if (mode == '-')
{
// Supprimer des modes
add = false;
modeChanges += "-";
} else if (mode == 'i')
{
// Mode invitation uniquement
channel->setMode(MODE_INVITE_ONLY, add);
modeChanges += "i";
}
else if (mode == 't')
{
// Mode restriction du topic
channel->setMode(MODE_TOPIC_LOCKED, add);
modeChanges += "t";
}
 else if (mode == 'k')
{
// Mode mot de passe
if (add)
{
// Vérifier s'il y a assez de paramètres
if (paramIndex >= modeParams.size())
{
client->sendReply("461 MODE :Not enough parameters");
continue;
}

// Définir le mot de passe
channel->setPassword(modeParams[paramIndex]);
channel->setMode(MODE_PASSWORD, true);

// Ajouter au message MODE
modeChanges += "k";
paramChanges += " " + modeParams[paramIndex];

paramIndex++;
}
else
{
// Supprimer le mot de passe
channel->setPassword("");
channel->setMode(MODE_PASSWORD, false);

// Ajouter au message MODE
modeChanges += "k";
}
}
else if (mode == 'l')
{
// Mode limite d'utilisateurs
if (add)
{
// Vérifier s'il y a assez de paramètres
if (paramIndex >= modeParams.size())
{
client->sendReply("461 MODE :Not enough parameters");
continue;
}

// Convertir le paramètre en entier
int limit = Utils::toInt(modeParams[paramIndex]);

// Vérifier que la limite est valide
if (limit <= 0)
{
client->sendReply("501 :Invalid user limit");
continue;
}

// Définir la limite d'utilisateurs
channel->setUserLimit(limit);
channel->setMode(MODE_USER_LIMIT, true);

// Ajouter au message MODE
modeChanges += "l";
paramChanges += " " + modeParams[paramIndex];

paramIndex++;
}
else
{
// Supprimer la limite d'utilisateurs
channel->setUserLimit(0);
channel->setMode(MODE_USER_LIMIT, false);

// Ajouter au message MODE
modeChanges += "l";
}
}
else if (mode == 'o')
{
// Mode opérateur
if (paramIndex >= modeParams.size())
{
client->sendReply("461 MODE :Not enough parameters");
continue;
}

// Rechercher la cible
Client* target = _server->getClientByNickname(modeParams[paramIndex]);

// Vérifier si la cible existe
if (target == NULL)
{
client->sendReply("401 " + modeParams[paramIndex] + " :No such nick/channel");
continue;
}

// Vérifier si la cible est dans le canal
if (!channel->hasClient(target))
{
client->sendReply("441 " + modeParams[paramIndex] + " " + channelName + " :They aren't on that channel");
continue;
}

// Définir/supprimer le statut d'opérateur
channel->setOperator(target, add);

// Ajouter au message MODE
modeChanges += "o";
paramChanges += " " + modeParams[paramIndex];

paramIndex++;
}
else
{
// Mode inconnu
client->sendReply("472 " + std::string(1, mode) + " :is unknown mode char to me");
}
}

// Envoyer un message MODE à tous les clients du canal
std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" +
client->getHostname() + " MODE " + channelName + " " +
modeChanges + paramChanges;
channel->broadcast(message, NULL);

// Log de changement de mode
Utils::logMessage("Modes du canal " + channelName + " changés par " + client->getNickname() +
": " + modeChanges + paramChanges);
}

void ModeCommand::handleUserMode(Client* client, const std::string& targetNick, const std::string& modes)
{
// Vérifier si le client cible lui-même
if (targetNick != client->getNickname())
{
// Client ne peut pas changer les modes des autres utilisateurs
client->sendReply("502 :Cannot change mode for other users");
return;
}

// Si pas de modes, afficher les modes actuels
if (modes.empty())
{
// Construire la chaîne des modes
std::string modeStr = "+";
if (client->isOperator()) modeStr += "o";

// Envoyer les modes
client->sendReply("221 " + modeStr);
return;
}

// Analyser les modes
bool add = true;  // Par défaut, on ajoute des modes

for (size_t i = 0; i < modes.size(); ++i)
{
char mode = modes[i];

if (mode == '+')
{
// Ajouter des modes
add = true;
} else if (mode == '-')
{
// Supprimer des modes
add = false;
}
else if (mode == 'o')
{
// Mode opérateur (ne peut pas être défini par l'utilisateur)
if (add)
{
client->sendReply("501 :Cannot set user mode +o");
}
else
{
// Un opérateur peut renoncer à son statut
if (client->isOperator()) {
client->setOperator(false);

// Envoyer un message MODE
std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" +
		   client->getHostname() + " MODE " + client->getNickname() + " -o";
client->sendMessage(message);

// Log de changement de mode
Utils::logMessage("Client " + client->getNickname() + " a renoncé à son statut d'opérateur");
}
}
}
else
{
// Mode inconnu
client->sendReply("501 :Unknown MODE flag");
}
}
}

// Implémentation de la commande TOPIC
TopicCommand::TopicCommand(Server* server)
: Command(server, "TOPIC", true, 1)
{
// vide
}

void TopicCommand::execute(Client* client, const std::vector<std::string>& params)
{
// Récupérer le nom du canal
std::string channelName = params[0];

// Rechercher le canal
Channel* channel = _server->getChannel(channelName);

// Vérifier si le canal existe
if (channel == NULL)
{
// Canal inexistant
client->sendReply("403 " + channelName + " :No such channel");
return;
}

// Vérifier si le client est dans le canal
if (!channel->hasClient(client))
{
// Client pas dans le canal
client->sendReply("442 " + channelName + " :You're not on that channel");
return;
}

// Si pas de paramètre supplémentaire, afficher le sujet actuel
if (params.size() == 1)
{
// Vérifier si le canal a un sujet
if (channel->getTopic().empty())
{
// Pas de sujet
client->sendReply("331 " + channelName + " :No topic is set");
}
else
{
// Envoyer le sujet
client->sendReply("332 " + channelName + " :" + channel->getTopic());
}
return;
}

// Récupérer le nouveau sujet
std::string newTopic = params[1];

// Vérifier si le client peut changer le sujet
if (!channel->clientCanChangeTopic(client))
{
// Client ne peut pas changer le sujet
client->sendReply("482 " + channelName + " :You're not channel operator");
return;
}

// Définir le nouveau sujet
channel->setTopic(newTopic, client);

// Envoyer un message TOPIC à tous les clients du canal
std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" +
client->getHostname() + " TOPIC " + channelName + " :" + newTopic;
channel->broadcast(message, NULL);

// Log de changement de sujet
Utils::logMessage("Sujet du canal " + channelName + " changé par " + client->getNickname() +
": " + newTopic);
}

// Implémentation de la commande KICK
KickCommand::KickCommand(Server* server)
: Command(server, "KICK", true, 2)
{
// vide
}

void KickCommand::execute(Client* client, const std::vector<std::string>& params)
{
// Récupérer le nom du canal et le pseudo de la cible
std::string channelName = params[0];
std::string targetNick = params[1];

// Récupérer le message de kick (optionnel)
std::string kickMessage = params.size() > 2 ? params[2] : "No reason given";

// Rechercher le canal
Channel* channel = _server->getChannel(channelName);

// Vérifier si le canal existe
if (channel == NULL)
{
// Canal inexistant
client->sendReply("403 " + channelName + " :No such channel");
return;
}

// Vérifier si le client est dans le canal
if (!channel->hasClient(client))
{
// Client pas dans le canal
client->sendReply("442 " + channelName + " :You're not on that channel");
return;
}

// Vérifier si le client est opérateur du canal
if (!channel->isOperator(client))
{
// Client pas opérateur
client->sendReply("482 " + channelName + " :You're not channel operator");
return;
}

// Rechercher la cible
Client* target = _server->getClientByNickname(targetNick);

// Vérifier si la cible existe
if (target == NULL)
{
// Cible inexistante
client->sendReply("401 " + targetNick + " :No such nick/channel");
return;
}

// Vérifier si la cible est dans le canal
if (!channel->hasClient(target))
{
// Cible pas dans le canal
client->sendReply("441 " + targetNick + " " + channelName + " :They aren't on that channel");
return;
}

// Envoyer un message KICK à tous les clients du canal
std::string message = ":" + client->getNickname() + "!" + client->getUsername() + "@" +
client->getHostname() + " KICK " + channelName + " " +
targetNick + " :" + kickMessage;
channel->broadcast(message, NULL);

// Supprimer la cible du canal
channel->removeClient(target);

// Log de kick
Utils::logMessage("Client " + targetNick + " a été kické du canal " + channelName +
" par " + client->getNickname() + ": " + kickMessage);
}

// Implémentation de la commande INVITE
InviteCommand::InviteCommand(Server* server)
: Command(server, "INVITE", true, 2)
{
// vide
}

void InviteCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter INVITE
}

// Implémentation de la commande NAMES
NamesCommand::NamesCommand(Server* server)
: Command(server, "NAMES", true, 0)
{
// vide
}

void NamesCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter NAMES
}

// Implémentation de la commande LIST
ListCommand::ListCommand(Server* server)
: Command(server, "LIST", true, 0)
{
// vide
}

void ListCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter LIST
}

// Implémentation de la commande PING
PingCommand::PingCommand(Server* server)
: Command(server, "PING", false, 1)
{
// vide
}

void PingCommand::execute(Client* client, const std::vector<std::string>& params)
{
// Répondre avec PONG
client->sendMessage(":" + _server->getServerName() + " PONG " + _server->getServerName() + " :" + params[0]);
}

// Implémentation de la commande PONG
PongCommand::PongCommand(Server* server)
: Command(server, "PONG", false, 0)
{
// vide
}

void PongCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// Rien à faire, simplement marquer que le client a répondu
}

// Implémentation de la commande AWAY
AwayCommand::AwayCommand(Server* server)
: Command(server, "AWAY", true, 0)
{
// vide
}

void AwayCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter AWAY
}

// Implémentation de la commande WHO
WhoCommand::WhoCommand(Server* server)
: Command(server, "WHO", true, 0)
{
// vide
}

void WhoCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter WHO
}

// Implémentation de la commande WHOIS
WhoisCommand::WhoisCommand(Server* server)
: Command(server, "WHOIS", true, 1)
{
// vide
}

void WhoisCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter WHOIS
}

// Implémentation de la commande OPER
OperCommand::OperCommand(Server* server)
: Command(server, "OPER", true, 2)
{
// vide
}

void OperCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter OPER
}

// Implémentation de la commande FILE (bonus)
FileCommand::FileCommand(Server* server)
: Command(server, "FILE", true, 2)
{
// vide
}

void FileCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter FILE (bonus)
}

// Implémentation de la commande BOT (bonus)
BotCommand::BotCommand(Server* server)
: Command(server, "BOT", true, 1)
{
// vide
}

void BotCommand::execute(Client* client, const std::vector<std::string>& params)
{
(void)client; // Pour éviter l'avertissement de paramètre non utilisé
(void)params; // Pour éviter l'avertissement de paramètre non utilisé

// TODO: Implémenter BOT (bonus)
}