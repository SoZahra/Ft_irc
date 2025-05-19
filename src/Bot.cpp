#include "../includes/Bot.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Utils.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cctype>

/**
 * Constructeur de la classe Bot
 * arg server Pointeur vers le serveur
 */
Bot::Bot(Server* server)
    : _server(server),            // Initialiser le serveur
      _nickname("IRCBot"),        // Pseudo par défaut
      _username("bot"),           // Nom d'utilisateur par défaut
      _realname("IRC Bot"),       // Nom réel par défaut
      _botClient(NULL),           // Client initialement NULL
      _active(false)              // Bot initialement inactif
{
    // Charger les réponses préprogrammées
    loadResponses();

    // Log de création du bot
    Utils::logMessage("Bot IRC créé");
}

/**
 * Destructeur de la classe Bot
 */
Bot::~Bot()
{
    // Log de destruction du bot
    Utils::logMessage("Bot IRC détruit");

    // Nettoyer les cooldowns
    _cooldowns.clear();

    // Nettoyer les réponses
    _responses.clear();
}

/**
 * Initialise le bot
 * return true si l'initialisation a réussi, false sinon
 */
bool Bot::init()
{
    // Créer un client pour le bot
    _botClient = new Client(-1, _server);

    // Configurer le client
    _botClient->setNickname(_nickname);
    _botClient->setUsername(_username);
    _botClient->setRealname(_realname);
    _botClient->setStatus(REGISTERED);

    // Log d'initialisation
    Utils::logMessage("Bot IRC initialisé: " + _nickname);

    return true;
}

/**
 * Définit le pseudo du bot
 * arg nickname Nouveau pseudo
 */
void Bot::setNickname(const std::string& nickname)
{
    // Sauvegarder l'ancien pseudo
    std::string oldNickname = _nickname;

    // Mettre à jour le pseudo
    _nickname = nickname;

    // Mettre à jour le client si existant
    if (_botClient)
	{
        _botClient->setNickname(nickname);
    }

    // Log de changement de pseudo
    Utils::logMessage("Bot IRC: pseudo changé de " + oldNickname + " à " + _nickname);
}

/**
 * Définit le nom d'utilisateur du bot
 * arg username Nouveau nom d'utilisateur
 */
void Bot::setUsername(const std::string& username)
{
    // Mettre à jour le nom d'utilisateur
    _username = username;

    // Mettre à jour le client si existant
    if (_botClient)
	{
        _botClient->setUsername(username);
    }

    // Log de changement de nom d'utilisateur
    Utils::logMessage("Bot IRC: nom d'utilisateur changé en " + _username);
}

/**
 * Définit le nom réel du bot
 * arg realname Nouveau nom réel
 */
void Bot::setRealname(const std::string& realname)
{
    // Mettre à jour le nom réel
    _realname = realname;

    // Mettre à jour le client si existant
    if (_botClient)
	{
        _botClient->setRealname(realname);
    }

    // Log de changement de nom réel
    Utils::logMessage("Bot IRC: nom réel changé en " + _realname);
}

/**
 * Vérifie si le bot est actif
 * return true si le bot est actif, false sinon
 */
bool Bot::isActive() const
{
    return _active;
}

/**
 * Active le bot
 */
void Bot::activate()
{
    // Activer le bot
    _active = true;

    // Log d'activation
    Utils::logMessage("Bot IRC activé");
}

/**
 * Désactive le bot
 */
void Bot::deactivate()
{
    // Désactiver le bot
    _active = false;

    // Log de désactivation
    Utils::logMessage("Bot IRC désactivé");
}

/**
 * Charge les réponses préprogrammées depuis un fichier de configuration
 */
void Bot::loadResponses()
{
    // Essayer d'ouvrir le fichier de configuration
    std::ifstream file("bot_config.txt");

    // Si le fichier existe, charger les réponses
    if (file.is_open())
	{
        std::string line;

        while (std::getline(file, line))
		{
            // Ignorer les lignes vides ou les commentaires
            if (line.empty() || line[0] == '#')
			{
                continue;
            }

            // Format: trigger|response|exactMatch
            std::vector<std::string> parts = Utils::split(line, '|');

            // Vérifier qu'il y a au moins 2 parties
            if (parts.size() >= 2)
			{
                BotResponse response;
                response.trigger = parts[0];
                response.response = parts[1];
                response.exactMatch = parts.size() > 2 ? (parts[2] == "true" || parts[2] == "1") : false;

                // Ajouter la réponse à la liste
                _responses.push_back(response);
            }
        }

        // Fermer le fichier
        file.close();

        // Log de chargement des réponses
        Utils::logMessage("Bot IRC: " + Utils::toString(_responses.size()) + " réponses chargées");
    }

	else
	{
        // Ajouter quelques réponses par défaut
        // _responses.push_back((BotResponse){"bonjour", "Bonjour ! Comment puis-je vous aider ?", false});
        // _responses.push_back((BotResponse){"salut", "Salut ! Je suis le bot du serveur.", false});
        // _responses.push_back((BotResponse){"help", "Commandes disponibles: !time, !weather <ville>, !calc <expression>, !define <mot>, !joke, !stats", true});
        // _responses.push_back((BotResponse){"merci", "De rien ! Je suis là pour aider.", false});

        // pas compatible avec cpp98, version compatible ;
        BotResponse reponse1 = {"bonjour", "Bonjour ! Comment puis-je vous aider ?", false};
        _responses.push_back(reponse1);
        BotResponse reponse2 = {"salut", "Salut ! Je suis le bot du serveur.", false};
        _responses.push_back(reponse2);
        BotResponse reponse3 = {"help", "Commandes disponibles: !time, !weather <ville>, !calc <expression>, !define <mot>, !joke, !stats", true};
        _responses.push_back(reponse3);
        BotResponse reponse4 = {"merci", "De rien ! Je suis là pour aider.", false};
        _responses.push_back(reponse4);

        // Log d'utilisation des réponses par défaut
        Utils::logMessage("Bot IRC: Fichier de configuration non trouvé, utilisation des réponses par défaut");
    }
}

/**
 * Vérifie si une commande est en cooldown
 * arg command Nom de la commande
 * return true si la commande est en cooldown, false sinon
 */
bool Bot::isOnCooldown(const std::string& command)
{
    // Vérifier si la commande est dans le map des cooldowns
    std::map<std::string, time_t>::iterator it = _cooldowns.find(command);

    // Si la commande n'est pas dans le map, elle n'est pas en cooldown
    if (it == _cooldowns.end())
	{
        return false;
    }

    // Vérifier si le cooldown est expiré
    time_t now = std::time(NULL);
    if (now - it->second > 5)
	{  // 5 secondes de cooldown par défaut
        // Supprimer le cooldown expiré
        _cooldowns.erase(it);
        return false;
    }

    // La commande est en cooldown
    return true;
}

/**
 * Définit un cooldown pour une commande
 * arg command Nom de la commande
 * arg seconds Durée du cooldown en secondes
 */
void Bot::setCooldown(const std::string& command, int seconds)
{
    (void)seconds; // Pour éviter l'avertissement de paramètre non utilisé

    // Définir le cooldown (temps actuel)
    _cooldowns[command] = std::time(NULL);
}

/**
 * Remplace les variables dans un message
 * arg message Message contenant des variables
 * arg client Client concerné
 * return Message avec les variables remplacées
 */
std::string Bot::processVariables(const std::string& message, Client* client)
{
    // Si le message est vide ou pas de client, retourner le message tel quel
    if (message.empty() || !client)
	{
        return message;
    }

    // Copier le message
    std::string result = message;

    // Remplacer les variables
    std::string nickname = client->getNickname();
    size_t pos;

    // $NICK -> pseudo du client
    while ((pos = result.find("$NICK")) != std::string::npos)
	{
        result.replace(pos, 5, nickname);
    }

    // $TIME -> heure actuelle
    while ((pos = result.find("$TIME")) != std::string::npos)
	{
        result.replace(pos, 5, Utils::getCurrentTime());
    }

    // $SERVER -> nom du serveur
    while ((pos = result.find("$SERVER")) != std::string::npos)
	{
        result.replace(pos, 7, _server->getServerName());
    }

    // $BOTNAME -> nom du bot
    while ((pos = result.find("$BOTNAME")) != std::string::npos)
	{
        result.replace(pos, 8, _nickname);
    }

    return result;
}

/**
 * Traite une commande envoyée au bot
 * arg client Client qui envoie la commande
 * arg command Nom de la commande
 * arg params Paramètres de la commande
 */
void Bot::processCommand(Client* client, const std::string& command, const std::vector<std::string>& params)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier que le bot est actif
    if (!_active)
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Je suis désactivé. Demandez à un opérateur de m'activer.");
        return;
    }

    // Traiter la commande
    if (command == "help" || command == "!help")
	{
        // Afficher l'aide
        help(client);
    }

	else if (command == "weather" || command == "!weather")
	{
        // Vérifier qu'il y a un paramètre
        if (params.empty())
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !weather <ville>");
            return;
        }

        // Afficher la météo
        weather(client, params[0]);
    }

	else if (command == "calculate" || command == "!calc" || command == "!calculate")
	{
        // Vérifier qu'il y a un paramètre
        if (params.empty())
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !calc <expression>");
            return;
        }

        // Calculer l'expression
        calculate(client, params[0]);
    }

	else if (command == "define" || command == "!define")
	{
        // Vérifier qu'il y a un paramètre
        if (params.empty())
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !define <mot>");
            return;
        }

        // Définir le mot
        define(client, params[0]);
    }

	else if (command == "time" || command == "!time")
	{
        // Afficher l'heure
        time(client);
    }

	else if (command == "joke" || command == "!joke")
	{
        // Raconter une blague
        joke(client);
    }

	else if (command == "stats" || command == "!stats")
	{
        // Afficher les statistiques
        stats(client);
    }

	else if (command == "activate" || command == "!activate")
	{
        // Vérifier que le client est un opérateur
        if (client->isOperator())
		{
            // Activer le bot
            activate();
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Bot activé");
        }

		else
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vous n'avez pas les droits pour activer le bot");
        }
    }

	else if (command == "deactivate" || command == "!deactivate")
	{
        // Vérifier que le client est un opérateur
        if (client->isOperator())
		{
            // Désactiver le bot
            deactivate();
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Bot désactivé");
        }
		else
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vous n'avez pas les droits pour désactiver le bot");
        }
    }

	else if (command == "join" || command == "!join")
	{
        // Vérifier que le client est un opérateur
        if (client->isOperator())
		{
            // Vérifier qu'il y a un paramètre
            if (params.empty())
			{
                client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !join <canal>");
                return;
            }

            // Rejoindre le canal
            joinChannel(params[0]);
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Bot a rejoint le canal " + params[0]);
        }
		else
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vous n'avez pas les droits pour faire rejoindre un canal au bot");
        }
    }

	else if (command == "leave" || command == "!leave")
	{
        // Vérifier que le client est un opérateur
        if (client->isOperator())
		{
            // Vérifier qu'il y a un paramètre
            if (params.empty())
			{
                client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !leave <canal>");
                return;
            }

            // Quitter le canal
            leaveChannel(params[0]);
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Bot a quitté le canal " + params[0]);
        }
		else
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vous n'avez pas les droits pour faire quitter un canal au bot");
        }

    }

	else if (command == "say" || command == "!say")
	{
        // Vérifier que le client est un opérateur
        if (client->isOperator())
		{
            // Vérifier qu'il y a au moins deux paramètres
            if (params.size() < 2)
			{
                client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Utilisation: !say <canal/pseudo> <message>");
                return;
            }

            // Récupérer la cible et le message
            std::string target = params[0];
            std::string message = params[1];

            // Envoyer le message
            if (target[0] == '#' || target[0] == '&')
			{
                // Message à un canal
                sayToChannel(target, message);
                client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Message envoyé au canal " + target);
            }
			else
			{
                // Message à un utilisateur
                sayToUser(target, message);
                client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Message envoyé à " + target);
            }
        }
		else
		{
            client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vous n'avez pas les droits pour faire parler le bot");
        }
    }

	else
	{
        // Commande inconnue
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Commande inconnue. Tapez !help pour voir les commandes disponibles.");
    }
}

/**
 * Traite un message envoyé sur un canal
 * arg client Client qui envoie le message
 * arg channel Canal sur lequel le message est envoyé
 * arg message Contenu du message
 */
void Bot::processChannelMessage(Client* client, Channel* channel, const std::string& message)
{
    // Vérifier que le client et le canal existent
    if (!client || !channel)
	{
        return;
    }

    // Vérifier que le bot est actif
    if (!_active)
	{
        return;
    }

    // Vérifier si le bot est dans le canal
    if (!_botClient->isInChannel(channel))
	{
        return;
    }

    // Vérifier si le message est une commande (commence par !)
    if (message.size() > 1 && message[0] == '!')
	{
        // Extraire la commande
        std::string command;
        std::vector<std::string> params;

        // Trouver le premier espace
        size_t spacePos = message.find(' ');
        if (spacePos != std::string::npos)
		{
            // Extraire la commande et les paramètres
            command = message.substr(1, spacePos - 1);

            // Extraire les paramètres
            std::string paramsStr = message.substr(spacePos + 1);
            params = Utils::split(paramsStr, ' ');
        }
		else
		{
            // Pas de paramètres
            command = message.substr(1);
        }

        // Traiter la commande
        processCommand(client, command, params);
        return;
    }

    // Si ce n'est pas une commande, vérifier les réponses préprogrammées
    for (size_t i = 0; i < _responses.size(); ++i)
	{
        // Copier le trigger et le message pour la comparaison
        std::string trigger = _responses[i].trigger;
        std::string msgLower = message;

        // Convertir en minuscules pour une comparaison insensible à la casse
        std::transform(trigger.begin(), trigger.end(), trigger.begin(), ::tolower);
        std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);

        // Vérifier si le message contient le trigger
        if ((_responses[i].exactMatch && msgLower == trigger) ||
            (!_responses[i].exactMatch && msgLower.find(trigger) != std::string::npos))
		{
            // Envoyer la réponse
            std::string response = processVariables(_responses[i].response, client);
            sayToChannel(channel->getName(), response);
            return;
        }
    }
}

/**
 * Traite un message privé envoyé au bot
 * arg client Client qui envoie le message
 * arg message Contenu du message
 */
void Bot::processPrivateMessage(Client* client, const std::string& message)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier que le bot est actif
    if (!_active)
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Je suis désactivé. Demandez à un opérateur de m'activer.");
        return;
    }

    // Vérifier si le message est une commande (commence par ! ou pas)
    if (message.size() > 1 && message[0] == '!')
	{
        // Extraire la commande
        std::string command;
        std::vector<std::string> params;

        // Trouver le premier espace
        size_t spacePos = message.find(' ');
        if (spacePos != std::string::npos)
		{
            // Extraire la commande et les paramètres
            command = message.substr(1, spacePos - 1);

            // Extraire les paramètres
            std::string paramsStr = message.substr(spacePos + 1);
            params = Utils::split(paramsStr, ' ');
        }
		else
		{
            // Pas de paramètres
            command = message.substr(1);
        }

        // Traiter la commande
        processCommand(client, command, params);
        return;
    }
	else
	{
        // Extraire la commande
        std::string command;
        std::vector<std::string> params;

        // Trouver le premier espace
        size_t spacePos = message.find(' ');
        if (spacePos != std::string::npos)
		{
            // Extraire la commande et les paramètres
            command = message.substr(0, spacePos);

            // Extraire les paramètres
            std::string paramsStr = message.substr(spacePos + 1);
            params = Utils::split(paramsStr, ' ');
        }
		else
		{
            // Pas de paramètres
            command = message;
        }

        // Traiter la commande
        processCommand(client, command, params);
        return;
    }
}

/**
 * Fait rejoindre un canal au bot
 * arg channelName Nom du canal à rejoindre
 */
void Bot::joinChannel(const std::string& channelName)
{
    // Vérifier que le bot est actif
    if (!_active || !_botClient)
	{
        return;
    }

    // Vérifier si le canal existe
    Channel* channel = _server->getChannel(channelName);

    // Si le canal n'existe pas, le créer
    if (channel == NULL)
	{
        channel = _server->createChannel(channelName, _botClient);
    }
	else
	{
        // Ajouter le bot au canal
        channel->addClient(_botClient, false);
    }

    // Log de rejointe de canal
    Utils::logMessage("Bot IRC a rejoint le canal " + channelName);

    // Envoyer un message de bienvenue
    sayToChannel(channelName, "Bonjour à tous ! Je suis " + _nickname + ", le bot du serveur.");
}

/**
 * Fait quitter un canal au bot
 * arg channelName Nom du canal à quitter
 */
void Bot::leaveChannel(const std::string& channelName)
{
    // Vérifier que le bot est actif
    if (!_active || !_botClient)
	{
        return;
    }

    // Vérifier si le canal existe
    Channel* channel = _server->getChannel(channelName);

    // Si le canal n'existe pas, ignorer
    if (channel == NULL)
	{
        return;
    }

    // Vérifier si le bot est dans le canal
    if (!_botClient->isInChannel(channel))
	{
        return;
    }

    // Envoyer un message d'au revoir
    sayToChannel(channelName, "Au revoir à tous !");

    // Supprimer le bot du canal
    channel->removeClient(_botClient);

    // Log de départ de canal
    Utils::logMessage("Bot IRC a quitté le canal " + channelName);
}

/**
 * Envoie un message sur un canal
 * arg channelName Nom du canal
 * arg message Message à envoyer
 */
void Bot::sayToChannel(const std::string& channelName, const std::string& message)
{
    // Vérifier que le bot est actif
    if (!_active || !_botClient)
	{
        return;
    }

    // Vérifier si le canal existe
    Channel* channel = _server->getChannel(channelName);

    // Si le canal n'existe pas, ignorer
    if (channel == NULL)
	{
        return;
    }

    // Vérifier si le bot est dans le canal
    if (!_botClient->isInChannel(channel))
	{
        return;
    }

    // Envoyer le message
    channel->broadcast(":" + _nickname + "!" + _username + "@localhost PRIVMSG " + channelName + " :" + message, _botClient);

    // Log d'envoi de message
    // Utils::logMessage("Bot IRC a envoyé un message au canal " + channelName + ": " + message);
}

/**
 * Envoie un message à un utilisateur
 * arg nickname Pseudo de l'utilisateur
 * arg message Message à envoyer
 */
void Bot::sayToUser(const std::string& nickname, const std::string& message)
{
    // Vérifier que le bot est actif
    if (!_active || !_botClient)
	{
        return;
    }

    // Rechercher l'utilisateur
    Client* client = _server->getClientByNickname(nickname);

    // Si l'utilisateur n'existe pas, ignorer
    if (client == NULL)
	{
        return;
    }

    // Envoyer le message
    client->sendMessage(":" + _nickname + "!" + _username + "@localhost PRIVMSG " + nickname + " :" + message);

    // Log d'envoi de message
    Utils::logMessage("Bot IRC a envoyé un message à " + nickname + ": " + message);
}

/**
 * Affiche l'aide du bot
 * arg client Client qui demande l'aide
 */
void Bot::help(Client* client)
{
    // Vérifier que le client existe
    if (!client) {
        return;
    }

    // Envoyer l'aide
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Commandes disponibles:");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!help - Affiche cette aide");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!time - Affiche l'heure actuelle");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!weather <ville> - Affiche la météo pour une ville");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!calc <expression> - Calcule une expression mathématique");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!define <mot> - Donne la définition d'un mot");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!joke - Raconte une blague");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!stats - Affiche les statistiques du serveur");

    // Si le client est un opérateur, ajouter les commandes admin
    if (client->isOperator()) {
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Commandes admin:");
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!activate - Active le bot");
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!deactivate - Désactive le bot");
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!join <canal> - Fait rejoindre un canal au bot");
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!leave <canal> - Fait quitter un canal au bot");
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :!say <canal/pseudo> <message> - Fait parler le bot");
    }
}

/**
 * Affiche la météo pour une ville
 * arg client Client qui demande la météo
 * arg location Ville pour laquelle afficher la météo
 */
void Bot::weather(Client* client, const std::string& location)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("weather"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("weather", 5);

    // En réalité, on devrait faire une requête à une API météo
    // Mais pour cet exemple, on va juste générer une météo aléatoire

    // Température entre -10 et 40 degrés
    int temperature = rand() % 51 - 10;

    // Conditions météo aléatoires
    std::string conditions[] = {"ensoleillé", "nuageux", "pluvieux", "orageux", "neigeux", "brumeux"};
    std::string condition = conditions[rand() % 6];

    // Humidité entre 0 et 100%
    int humidity = rand() % 101;

    // Vent entre 0 et 100 km/h
    int wind = rand() % 101;

    // Afficher la météo
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Météo pour " + location + ":");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Température: " + Utils::toString(temperature) + "°C");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Conditions: " + condition);
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Humidité: " + Utils::toString(humidity) + "%");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Vent: " + Utils::toString(wind) + " km/h");
}

/**
 * Calcule une expression mathématique
 * arg client Client qui demande le calcul
 * arg expression Expression à calculer
 */
void Bot::calculate(Client* client, const std::string& expression)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("calculate"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("calculate", 2);

    // Implémentation simplifiée d'un calculateur
    // Supporte seulement les opérations de base (+, -, *, /)

    try
	{
        // Analyser l'expression
        std::istringstream iss(expression);
        double result = 0.0;
        char op = '+';
        double value;

        // Lire la première valeur
        iss >> result;

        // Lire les opérations suivantes
        while (iss >> op >> value)
		{
            switch (op)
			{
                case '+':
                    result += value;
                    break;
                case '-':
                    result -= value;
                    break;
                case '*':
                    result *= value;
                    break;
                case '/':
                    if (value == 0)
					{
                        throw std::runtime_error("Division par zéro");
                    }
                    result /= value;
                    break;
                default:
                    throw std::runtime_error("Opérateur non supporté");
            }
        }

        // Afficher le résultat
        std::ostringstream oss;
        oss << result;
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :" + expression + " = " + oss.str());
    }
	catch (const std::exception& e)
	{
        // Erreur lors du calcul
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Erreur de calcul: " + std::string(e.what()));
    }
}

/**
 * Donne la définition d'un mot
 * arg client Client qui demande la définition
 * arg word Mot à définir
 */
void Bot::define(Client* client, const std::string& word)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("define"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("define", 5);

    // En réalité, on devrait faire une requête à une API de dictionnaire
    // Mais pour cet exemple, on va juste donner quelques définitions prédéfinies

    // Dictionnaire simplifié
    std::map<std::string, std::string> dictionary;
    dictionary["irc"] = "Internet Relay Chat, un protocole de communication textuelle sur Internet";
    dictionary["bot"] = "Programme informatique qui effectue des tâches automatiques";
    dictionary["serveur"] = "Ordinateur ou programme informatique qui fournit des services à d'autres ordinateurs";
    dictionary["canal"] = "Dans le contexte IRC, espace de discussion où les utilisateurs peuvent échanger des messages";
    dictionary["client"] = "Programme informatique qui accède à un service sur un autre ordinateur";
    dictionary["commande"] = "Instruction donnée à un programme pour effectuer une tâche spécifique";

    // Convertir le mot en minuscules pour la recherche
    std::string wordLower = word;
    std::transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::tolower);

    // Rechercher le mot dans le dictionnaire
    std::map<std::string, std::string>::iterator it = dictionary.find(wordLower);

    // Afficher la définition
    if (it != dictionary.end()) {
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Définition de \"" + word + "\" : " + it->second);
    }
	else
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Désolé, je ne connais pas la définition de \"" + word + "\"");
    }
}

/**
 * Affiche l'heure actuelle
 * arg client Client qui demande l'heure
 */
void Bot::time(Client* client)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("time"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("time", 1);

    // Récupérer l'heure actuelle
    std::string currentTime = Utils::getCurrentTime();

    // Afficher l'heure
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Heure actuelle: " + currentTime);
}

/**
 * Raconte une blague
 * arg client Client qui demande une blague
 */
void Bot::joke(Client* client)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("joke"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("joke", 5);

    // Blagues prédéfinies
    std::vector<std::string> jokes;
    jokes.push_back("Pourquoi les plongeurs plongent-ils toujours en arrière et jamais en avant ? Parce que sinon ils tombent dans le bateau.");
    jokes.push_back("C'est l'histoire d'un pingouin qui respire par les fesses. Un jour il s'assoit et il meurt.");
    jokes.push_back("Qu'est-ce qu'un crocodile qui surveille la pharmacie ? Un pharmaco-vigilant.");
    jokes.push_back("Que fait un crocodile quand il rencontre une superbe femelle ? Il Lacoste.");
    jokes.push_back("Quel est le comble pour un électricien ? De ne pas être au courant.");
    jokes.push_back("Pourquoi les éléphants n'utilisent pas d'ordinateur ? Parce qu'ils ont peur des souris.");
    jokes.push_back("Qu'est-ce qui est petit, carré et jaune ? Un petit carré jaune.");
    jokes.push_back("Un homme rentre dans un café. PLOUF !");

    // Choisir une blague au hasard
    int index = rand() % jokes.size();

    // Afficher la blague
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :" + jokes[index]);
}

/**
 * Affiche les statistiques du serveur
 * arg client Client qui demande les statistiques
 */
void Bot::stats(Client* client)
{
    // Vérifier que le client existe
    if (!client)
	{
        return;
    }

    // Vérifier si la commande est en cooldown
    if (isOnCooldown("stats"))
	{
        client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Cette commande est en cooldown. Veuillez réessayer dans quelques secondes.");
        return;
    }

    // Définir le cooldown
    setCooldown("stats", 5);

    // Récupérer les statistiques
    unsigned int clientCount = _server->getClientCount();
    unsigned int channelCount = _server->getChannelCount();

    // Afficher les statistiques
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Statistiques du serveur " + _server->getServerName() + ":");
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Clients connectés: " + Utils::toString(clientCount));
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Canaux actifs: " + Utils::toString(channelCount));
    client->sendMessage(":" + _nickname + " PRIVMSG " + client->getNickname() + " :Date de création du serveur: " + _server->getCreationDate());
}