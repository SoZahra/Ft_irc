#ifndef BOT_HPP
# define BOT_HPP

# include <string>        // Pour les chaînes de caractères
# include <map>           // Pour stocker les commandes du bot
# include <vector>        // Pour stocker les paramètres
# include <ctime>         // Pour les horodatages

# include "Server.hpp"
# include "Client.hpp"
# include "Channel.hpp"

class Server;
class Client;
class Channel;

// Structure pour stocker une réponse préprogrammée
struct BotResponse 
{
    std::string     trigger;        // Mot déclencheur
    std::string     response;       // Réponse à envoyer
    bool            exactMatch;     // True si le déclencheur doit correspondre exactement
};

class Bot 
{
private:
    Server*                 _server;            // Pointeur vers le serveur
    std::string             _nickname;          // Pseudo du bot
    std::string             _username;          // Nom d'utilisateur du bot
    std::string             _realname;          // Nom réel du bot
    Client*                 _botClient;         // Client représentant le bot
    std::vector<BotResponse> _responses;        // Réponses préprogrammées
    std::map<std::string, time_t> _cooldowns;   // Cooldowns des commandes
    bool                    _active;            // Si le bot est actif
    
    // Méthodes privées
    void loadResponses();                       // Charge les réponses du fichier de configuration
    bool isOnCooldown(const std::string& command); // Vérifie si une commande est en cooldown
    void setCooldown(const std::string& command, int seconds); // Définit un cooldown
    std::string processVariables(const std::string& message, Client* client); // Remplace les variables
    
public:
    // Constructeur et destructeur
    Bot(Server* server);
    ~Bot();
    
    // Initialisation et configuration
    bool init();
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    
    // Gestion de l'état du bot
    bool isActive() const;
    void activate();
    void deactivate();
    
    // Commandes du bot
    void processCommand(Client* client, const std::string& command, const std::vector<std::string>& params);
    void processChannelMessage(Client* client, Channel* channel, const std::string& message);
    void processPrivateMessage(Client* client, const std::string& message);
    
    // Actions du bot
    void joinChannel(const std::string& channelName);
    void leaveChannel(const std::string& channelName);
    void sayToChannel(const std::string& channelName, const std::string& message);
    void sayToUser(const std::string& nickname, const std::string& message);
    
    // Commandes utilitaires du bot
    void help(Client* client);
    void weather(Client* client, const std::string& location);
    void calculate(Client* client, const std::string& expression);
    void define(Client* client, const std::string& word);
    void time(Client* client);
    void joke(Client* client);
    void stats(Client* client);
};

#endif