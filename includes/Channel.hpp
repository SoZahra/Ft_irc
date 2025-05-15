#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>       // Pour les chaînes de caractères
# include <vector>       // Pour stocker des collections de données
# include <map>          // Pour stocker les clients et leurs modes
# include <set>          // Pour stocker des collections uniques

# include "Client.hpp"

class Client;

// Enumération des modes de canal
enum ChannelMode 
{
    MODE_INVITE_ONLY = 0x01,    // Mode invite (+i) - Canal sur invitation uniquement
    MODE_TOPIC_LOCKED = 0x02,   // Mode topic (+t) - Seuls les opérateurs peuvent changer le topic
    MODE_PASSWORD = 0x04,       // Mode key (+k) - Canal protégé par mot de passe
    MODE_USER_LIMIT = 0x08      // Mode limit (+l) - Limite d'utilisateurs
};

// Enumération des modes utilisateur dans un canal
enum UserMode 
{
    USER_MODE_OPERATOR = 0x01,     // Mode opérateur (+o) - Opérateur de canal
    USER_MODE_VOICE = 0x02         // Mode voice (+v) - Utilisateur avec droit de parole (bonus)
};

class Channel 
{
private:
    std::string                     _name;           // Nom du canal
    std::string                     _topic;          // Sujet du canal
    std::map<Client*, unsigned int> _clients;        // Clients dans le canal et leurs modes
    unsigned int                    _modes;          // Modes du canal
    std::string                     _password;       // Mot de passe du canal (si mode +k)
    unsigned int                    _userLimit;      // Limite d'utilisateurs (si mode +l)
    std::set<std::string>           _invitedUsers;   // Liste des utilisateurs invités (si mode +i)
    time_t                          _creationTime;   // Horodatage de création du canal
    
public:
    // Constructeur et destructeur
    Channel(const std::string& name, Client* creator);
    ~Channel();
    
    // Getters et setters
    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic, Client* setter);
    unsigned int getModes() const;
    const std::string& getPassword() const;
    void setPassword(const std::string& password);
    unsigned int getUserLimit() const;
    void setUserLimit(unsigned int limit);
    time_t getCreationTime() const;
    
    // Gestion des modes
    bool hasMode(ChannelMode mode) const;
    void setMode(ChannelMode mode, bool enabled);
    
    // Gestion des clients
    void addClient(Client* client, bool asOperator = false);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    bool hasClient(const std::string& nickname) const;
    Client* getClient(const std::string& nickname) const;
    std::vector<Client*> getClients() const;
    std::vector<Client*> getOperators() const;
    bool isOperator(Client* client) const;
    void setOperator(Client* client, bool op);
    bool hasVoice(Client* client) const;
    void setVoice(Client* client, bool voice);
    unsigned int getClientCount() const;
    
    // Gestion des invitations
    void inviteUser(const std::string& nickname);
    bool isInvited(const std::string& nickname) const;
    void removeInvite(const std::string& nickname);
    
    // Diffusion de messages
    void broadcast(const std::string& message, Client* exclude = NULL);
    
    // Vérifications de permissions
    bool clientCanJoin(Client* client, const std::string& password) const;
    bool clientCanChangeTopic(Client* client) const;
    
    // Conversion en string pour l'affichage
    std::string toString() const;
};

#endif