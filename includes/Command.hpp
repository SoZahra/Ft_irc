#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <string>        // Pour les chaînes de caractères
# include <vector>        // Pour stocker les paramètres
# include "Client.hpp"
# include "Server.hpp"

class Client;
class Server;
class Channel;

// Classe de base abstraite pour toutes les commandes IRC
class Command 
{
protected:
    Server*         _server;    // Pointeur vers le serveur (pour accéder aux clients, canaux, etc.)
    std::string     _name;      // Nom de la commande (ex: NICK, JOIN, etc.)
    bool            _requiresRegistration; // Si true, le client doit être enregistré pour utiliser la commande
    unsigned int    _minParams;  // Nombre minimum de paramètres

public:
    // Constructeur et destructeur
    Command(Server* server, const std::string& name, bool requiresRegistration, unsigned int minParams);
    virtual ~Command();
    
    // Getters
    const std::string& getName() const;
    bool requiresRegistration() const;
    unsigned int getMinParams() const;
    
    // Méthode pure virtuelle à implémenter par chaque commande
    virtual void execute(Client* client, const std::vector<std::string>& params) = 0;
};

// Commandes concrètes, chacune implémentant execute()

// PASS - Définit le mot de passe pour la connexion
class PassCommand : public Command 
{
public:
    PassCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// NICK - Définit ou change le pseudo d'un utilisateur
class NickCommand : public Command 
{
public:
    NickCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
    
    // Cette méthode doit être publique car elle est appelée depuis UserCommand
    void sendWelcomeMessages(Client* client);
};

// USER - Spécifie le nom d'utilisateur, l'hôte et le nom réel
class UserCommand : public Command 
{
public:
    UserCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// QUIT - Déconnecte du serveur
class QuitCommand : public Command 
{
public:
    QuitCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// JOIN - Rejoint un canal
class JoinCommand : public Command 
{
public:
    JoinCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
private:
    void sendNames(Client* client, Channel* channel);
};

// PART - Quitte un canal
class PartCommand : public Command 
{
public:
    PartCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// PRIVMSG - Envoie un message privé à un utilisateur ou un canal
class PrivmsgCommand : public Command 
{
public:
    PrivmsgCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// NOTICE - Envoie une notification à un utilisateur ou un canal
class NoticeCommand : public Command 
{
public:
    NoticeCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// MODE - Change le mode d'un canal ou d'un utilisateur
class ModeCommand : public Command 
{
public:
    ModeCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
    
private:
    void handleChannelMode(Client* client, const std::string& channelName, const std::string& modes, const std::vector<std::string>& params);
    void handleUserMode(Client* client, const std::string& targetNick, const std::string& modes);
};

// TOPIC - Affiche ou modifie le sujet d'un canal
class TopicCommand : public Command 
{
public:
    TopicCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// KICK - Éjecte un utilisateur d'un canal
class KickCommand : public Command 
{
public:
    KickCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// INVITE - Invite un utilisateur à rejoindre un canal
class InviteCommand : public Command 
{
public:
    InviteCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// NAMES - Liste les utilisateurs d'un canal
class NamesCommand : public Command 
{
public:
    NamesCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// LIST - Liste les canaux disponibles
class ListCommand : public Command 
{
public:
    ListCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// PING - Teste si le serveur est actif
class PingCommand : public Command 
{
public:
    PingCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// PONG - Réponse à un PING
class PongCommand : public Command 
{
public:
    PongCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// AWAY - Marque un utilisateur comme absent
class AwayCommand : public Command 
{
public:
    AwayCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// WHO - Demande des informations sur un utilisateur
class WhoCommand : public Command 
{
public:
    WhoCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// WHOIS - Demande des informations détaillées sur un utilisateur
class WhoisCommand : public Command 
{
public:
    WhoisCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// OPER - Donne les privilèges d'opérateur à un utilisateur
class OperCommand : public Command 
{
public:
    OperCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// Bonus - Commande pour le transfert de fichier
class FileCommand : public Command 
{
public:
    FileCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

// Bonus - Commande pour contrôler le bot
class BotCommand : public Command 
{
public:
    BotCommand(Server* server);
    virtual void execute(Client* client, const std::vector<std::string>& params);
};

#endif