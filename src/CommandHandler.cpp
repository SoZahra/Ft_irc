#include "../includes/CommandHandler.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Command.hpp"
#include "../includes/Utils.hpp"
#include <iomanip>      // Pour std::setw et std::setfill

/**
 * Constructeur de la classe CommandHandler
 * arg server Pointeur vers le serveur IRC
 */
CommandHandler::CommandHandler(Server* server)
    : _server(server) // Initialiser le pointeur vers le serveur
{
    // Enregistrer les commandes disponibles
    registerCommands();

    // Log de création du gestionnaire de commandes
    Utils::logMessage("Gestionnaire de commandes créé");
}

/**
 * Destructeur de la classe CommandHandler
 */
CommandHandler::~CommandHandler()
{
    // Supprimer toutes les commandes
    for (std::map<std::string, Command*>::iterator it = _commands.begin(); it != _commands.end(); ++it)
    {
        delete it->second;
    }
    _commands.clear();

    // Log de destruction du gestionnaire de commandes
    Utils::logMessage("Gestionnaire de commandes détruit");
}

/**
 * Enregistre toutes les commandes disponibles
 */
void CommandHandler::registerCommands()
{
    // Commandes d'authentification
    _commands["PASS"] = new PassCommand(_server);
    _commands["NICK"] = new NickCommand(_server);
    _commands["USER"] = new UserCommand(_server);

    // Commandes de base
    _commands["QUIT"] = new QuitCommand(_server);
    _commands["JOIN"] = new JoinCommand(_server);
    _commands["PART"] = new PartCommand(_server);
    _commands["PRIVMSG"] = new PrivmsgCommand(_server);
    _commands["NOTICE"] = new NoticeCommand(_server);

    // Commandes de canal
    _commands["MODE"] = new ModeCommand(_server);
    _commands["TOPIC"] = new TopicCommand(_server);
    _commands["KICK"] = new KickCommand(_server);
    _commands["INVITE"] = new InviteCommand(_server);
    _commands["NAMES"] = new NamesCommand(_server);
    _commands["LIST"] = new ListCommand(_server);

    // Commandes de serveur
    _commands["PING"] = new PingCommand(_server);
    _commands["PONG"] = new PongCommand(_server);
    _commands["AWAY"] = new AwayCommand(_server);
    _commands["WHO"] = new WhoCommand(_server);
    _commands["WHOIS"] = new WhoisCommand(_server);
    _commands["OPER"] = new OperCommand(_server);

    // Commandes bonus
    _commands["FILE"] = new FileCommand(_server);
    _commands["BOT"] = new BotCommand(_server);

    // Log d'enregistrement des commandes
    Utils::logMessage("Commandes enregistrées: " + Utils::toString(_commands.size()));
}

/**
 * Parse les paramètres d'une commande
 * arg paramsStr Chaîne de paramètres à parser
 * return Vecteur des paramètres parsés
 */
std::vector<std::string> CommandHandler::parseParams(const std::string& paramsStr)
{
    // Vecteur des paramètres
    std::vector<std::string> params;

    // Si la chaîne est vide, retourner un vecteur vide
    if (paramsStr.empty())
    {
        return params;
    }

    // Position actuelle dans la chaîne
    size_t pos = 0;

    // Tant qu'on n'a pas atteint la fin de la chaîne
    while (pos < paramsStr.size())
    {
        // Si on trouve un caractère ':' au début d'un paramètre, tout le reste est un seul paramètre
        if (paramsStr[pos] == ':')
        {
            params.push_back(paramsStr.substr(pos + 1));
            break;
        }

        // Ignorer les espaces
        while (pos < paramsStr.size() && isspace(paramsStr[pos]))
        {
            pos++;
        }

        // Si on a atteint la fin de la chaîne, sortir
        if (pos >= paramsStr.size())
        {
            break;
        }

        // Début du paramètre
        size_t start = pos;

        // Avancer jusqu'au prochain espace
        while (pos < paramsStr.size() && !isspace(paramsStr[pos]))
        {
            pos++;
        }

        // Ajouter le paramètre au vecteur
        params.push_back(paramsStr.substr(start, pos - start));
    }

    return params;
}

/**
 * Exécute une commande
 * arg client Client qui envoie la commande
 * arg message Message contenant la commande
 */
void CommandHandler::executeCommand(Client* client, const std::string& message)
{
    // Vérifier que le client existe
    if (!client)
    {
        return;
    }

    // Extraire le nom de la commande
    std::string cmdName;
    std::string params;

    // Trouver le premier espace
    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos)
    {
        // Extraire le nom de la commande et les paramètres
        cmdName = message.substr(0, spacePos);
        params = message.substr(spacePos + 1);
    }
    else
    {
        // Pas de paramètres
        cmdName = message;
        params = "";
    }

    // Convertir le nom de la commande en majuscules
    cmdName = Utils::toUpper(cmdName);

    // Rechercher la commande
    std::map<std::string, Command*>::iterator it = _commands.find(cmdName);
    if (it == _commands.end())
    {
        // Commande inconnue
        client->sendReply(formatReply(ERR_UNKNOWNCOMMAND, client, cmdName + " :Unknown command"));
        return;
    }

    // Récupérer la commande
    Command* cmd = it->second;

    // Vérifier si le client doit être enregistré pour utiliser cette commande
    if (cmd->requiresRegistration() && !client->isRegistered())
    {
        // Client non enregistré
        client->sendReply(formatReply(ERR_NOTREGISTERED, client, ":You have not registered"));
        return;
    }

    // Parser les paramètres
    std::vector<std::string> parsedParams = parseParams(params);

    // Vérifier le nombre de paramètres
    if (parsedParams.size() < cmd->getMinParams())
    {
        // Pas assez de paramètres
        client->sendReply(formatReply(ERR_NEEDMOREPARAMS, client, cmdName + " :Not enough parameters"));
        return;
    }

    // Exécuter la commande
    cmd->execute(client, parsedParams);
}

/**
 * Formate une réponse IRC
 * arg code Code de réponse
 * arg client Client concerné
 * arg message Message de la réponse
 * return Réponse formatée
 */
std::string CommandHandler::formatReply(int code, Client* client, const std::string& message)
{
    // Formater le code sur 3 chiffres
    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << code;

    // Ajouter le pseudo du client ou * si pas de pseudo
    ss << " " << (client->getNickname().empty() ? "*" : client->getNickname());

    // Ajouter le message
    ss << " " << message;

    return ss.str();
}

/**
 * Formate une réponse IRC avec plusieurs paramètres
 * arg code Code de réponse
 * arg client Client concerné
 * arg params Paramètres de la réponse
 * return Réponse formatée
 */
std::string CommandHandler::formatReply(int code, Client* client, const std::vector<std::string>& params)
{
    // Formater le code sur 3 chiffres
    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << code;

    // Ajouter le pseudo du client ou * si pas de pseudo
    ss << " " << (client->getNickname().empty() ? "*" : client->getNickname());

    // Ajouter les paramètres
    for (size_t i = 0; i < params.size(); ++i)
    {
        ss << " ";

        // Si c'est le dernier paramètre et qu'il contient des espaces, le préfixer par ':'
        if (i == params.size() - 1 && params[i].find(' ') != std::string::npos && params[i][0] != ':')
        {
            ss << ":";
        }

        ss << params[i];
    }

    return ss.str();
}

/**
 * Vérifie si un nom de canal est valide
 * arg name Nom de canal à vérifier
 * return true si le nom est valide, false sinon
 */
bool CommandHandler::isValidChannelName(const std::string& name)
{
    // Vérifier que le nom n'est pas vide
    if (name.empty())
    {
        return false;
    }

    // Vérifier que le nom commence par # ou &
    if (name[0] != '#' && name[0] != '&')
    {
        return false;
    }

    // Vérifier que le nom ne contient pas de caractères interdits
    for (size_t i = 1; i < name.size(); ++i)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
        {
            return false;
        }
    }

    return true;
}

/**
 * Vérifie si un pseudo est valide
 * arg nickname Pseudo à vérifier
 * return true si le pseudo est valide, false sinon
 */
bool CommandHandler::isValidNickname(const std::string& nickname)
{
    // Vérifier que le pseudo n'est pas vide
    if (nickname.empty())
    {
        return false;
    }

    // Vérifier que le pseudo ne commence pas par un chiffre ou un caractère spécial
    if (isdigit(nickname[0]) || nickname[0] == '-' || nickname[0] == '#' || nickname[0] == '&')
    {
        return false;
    }

    // Vérifier que le pseudo ne contient pas de caractères interdits
    for (size_t i = 0; i < nickname.size(); ++i)
    {
        if (nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' ||
            nickname[i] == '?' || nickname[i] == '!' || nickname[i] == '@' ||
            nickname[i] == '.' || nickname[i] == '$' || nickname[i] == ':')
        {
            return false;
        }
    }

    return true;
}