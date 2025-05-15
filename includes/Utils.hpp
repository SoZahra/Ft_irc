#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>       // Pour les chaînes de caractères
# include <vector>       // Pour les collections
# include <sstream>      // Pour les flux de chaînes

// Espace de noms pour les fonctions utilitaires
namespace Utils 
{
    // Fonctions pour manipuler les chaînes de caractères
    std::string             trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string             toLower(const std::string& str);
    std::string             toUpper(const std::string& str);
    
    // Fonctions pour les conversions
    int                     toInt(const std::string& str);
    std::string             toString(int value);
    
    // Fonctions pour la gestion du temps
    std::string             getCurrentTime();
    std::string             formatTime(time_t time);
    
    // Fonction de log
    void                    logMessage(const std::string& message, bool isError = false);
    
    // Fonctions pour les validations
    bool                    isValidChannelName(const std::string& name);
    bool                    isValidNickname(const std::string& nickname);
    
    // Fonctions pour générer des messages IRC
    std::string             formatIRCMessage(const std::string& prefix, const std::string& command, const std::vector<std::string>& params);
    std::vector<std::string> parseIRCMessage(const std::string& message, std::string& prefix, std::string& command);
    
    // Fonctions pour le réseau
    std::string             getHostname();
    std::string             getIPFromFd(int fd);
    
    // Fonctions pour la génération de chaînes aléatoires
    std::string             generateRandomString(int length);
    
    // Fonctions pour les fichiers
    bool                    fileExists(const std::string& path);
    std::string             getFileExtension(const std::string& filename);
    size_t                  getFileSize(const std::string& path);
    
    // Fonctions pour sécuriser les entrées
    std::string             sanitizeInput(const std::string& input);
}

#endif