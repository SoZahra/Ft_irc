#include "../includes/Utils.hpp"      // Fonctions utilitaires
#include <algorithm>    // Pour std::transform
#include <ctime>        // Pour time_t et struct tm
#include <cctype>       // Pour isspace, toupper, tolower
#include <fstream>      // Pour std::ifstream
#include <sys/stat.h>   // Pour struct stat
#include <unistd.h>     // Pour gethostname
#include <arpa/inet.h>  // Pour inet_ntop
#include <cstdlib>      // Pour rand, srand
#include <cstring>      // Pour strlen
#include <netdb.h>      // Pour getnameinfo
#include <iostream>     // Pour std::cout et std::cerr

namespace Utils
{

    /**
     * Supprime les espaces en début et fin de chaîne
     * arg str Chaîne à traiter
     * return Chaîne sans espaces au début et à la fin
     */
    std::string trim(const std::string& str)
    {
        // Si la chaîne est vide, on retourne une chaîne vide
        if (str.empty())
        {
            return str;
        }

        // Trouver le premier caractère non-espace
        size_t first = str.find_first_not_of(" \t\r\n");

        // Si la chaîne ne contient que des espaces, on retourne une chaîne vide
        if (first == std::string::npos)
        {
            return "";
        }

        // Trouver le dernier caractère non-espace
        size_t last = str.find_last_not_of(" \t\r\n");

        // Retourner la sous-chaîne
        return str.substr(first, last - first + 1);
    }

    /**
     * Divise une chaîne en un vecteur de sous-chaînes selon un délimiteur
     * arg str Chaîne à diviser
     * arg delimiter Caractère délimiteur
     * return Vecteur des sous-chaînes
     */
    std::vector<std::string> split(const std::string& str, char delimiter)
    {
        // Vecteur résultat
        std::vector<std::string> result;

        // Si la chaîne est vide, on retourne un vecteur vide
        if (str.empty())
        {
            return result;
        }

        // Utiliser un flux de chaînes pour diviser la chaîne
        std::stringstream ss(str);
        std::string item;

        // Lire chaque élément jusqu'au délimiteur
        while (std::getline(ss, item, delimiter))
        {
            // Ajouter l'élément au vecteur s'il n'est pas vide
            if (!item.empty())
            {
                result.push_back(item);
            }
        }

        return result;
    }

    /**
     * Convertit une chaîne en minuscules
     * arg str Chaîne à convertir
     * return Chaîne en minuscules
     */
    std::string toLower(const std::string& str)
    {
        // Si la chaîne est vide, on retourne une chaîne vide
        if (str.empty())
        {
            return str;
        }

        // Créer une copie de la chaîne
        std::string result = str;

        // Convertir chaque caractère en minuscule
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);

        return result;
    }

    /**
     * Convertit une chaîne en majuscules
     * arg str Chaîne à convertir
     * return Chaîne en majuscules
     */
    std::string toUpper(const std::string& str)
    {
        // Si la chaîne est vide, on retourne une chaîne vide
        if (str.empty())
        {
            return str;
        }

        // Créer une copie de la chaîne
        std::string result = str;

        // Convertir chaque caractère en majuscule
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);

        return result;
    }

    /**
     * Convertit une chaîne en entier
     * arg str Chaîne à convertir
     * return Valeur entière
     */
    int toInt(const std::string& str)
    {
        // Si la chaîne est vide, on retourne 0
        if (str.empty())
        {
            return 0;
        }

        // Utiliser un flux de chaînes pour convertir la chaîne en entier
        std::stringstream ss(str);
        int result;

        // Si la conversion échoue, on retourne 0
        if (!(ss >> result))
        {
            return 0;
        }

        return result;
    }

    /**
     * Convertit un entier en chaîne
     * arg value Valeur entière à convertir
     * return Chaîne représentant l'entier
     */
    std::string toString(int value)
    {
        // Utiliser un flux de chaînes pour convertir l'entier en chaîne
        std::stringstream ss;
        ss << value;

        return ss.str();
    }

    /**
     * Récupère l'heure actuelle au format "YYYY-MM-DD HH:MM:SS"
     * return Chaîne représentant l'heure actuelle
     */
    std::string getCurrentTime()
    {
        // Obtenir l'heure actuelle
        time_t now = time(NULL);
        struct tm* timeinfo = localtime(&now);

        // Formater l'heure
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        return std::string(buffer);
    }

    /**
     * Formate une heure au format "YYYY-MM-DD HH:MM:SS"
     * arg time Heure à formater
     * return Chaîne représentant l'heure
     */
    std::string formatTime(time_t time)
    {
        // Convertir le temps en structure tm
        struct tm* timeinfo = localtime(&time);

        // Formater l'heure
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        return std::string(buffer);
    }

    /**
     * Affiche un message de log
     * arg message Message à afficher
     * arg isError true si c'est une erreur, false sinon
     */
    void logMessage(const std::string& message, bool isError)
    {
        // Obtenir l'heure actuelle
        std::string time = getCurrentTime();

        // Afficher le message
        if (isError)
        {
            std::cerr << "[" << time << "] ERROR: " << message << std::endl;
        }
        else
        {
            std::cout << "[" << time << "] INFO: " << message << std::endl;
        }
    }

    /**
     * Vérifie si un nom de canal est valide
     * arg name Nom de canal à vérifier
     * return true si le nom est valide, false sinon
     */
    bool isValidChannelName(const std::string& name)
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
    bool isValidNickname(const std::string& nickname)
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

    /**
     * Formate un message IRC
     * arg prefix Préfixe du message
     * arg command Commande du message
     * arg params Paramètres du message
     * return Message IRC formaté
     */
    std::string formatIRCMessage(const std::string& prefix, const std::string& command, const std::vector<std::string>& params)
    {
        // Flux de chaînes pour construire le message
        std::stringstream ss;

        // Ajouter le préfixe s'il existe
        if (!prefix.empty())
        {
            ss << ":" << prefix << " ";
        }

        // Ajouter la commande
        ss << command;

        // Ajouter les paramètres
        for (size_t i = 0; i < params.size(); ++i)
        {
            // Si c'est le dernier paramètre et qu'il contient des espaces, le préfixer par ':'
            if (i == params.size() - 1 && params[i].find(' ') != std::string::npos && params[i][0] != ':')
            {
                ss << " :" << params[i];
            }
            else
            {
                ss << " " << params[i];
            }
        }

        return ss.str();
    }

    /**
     * Parse un message IRC
     * arg message Message à parser
     * arg prefix Préfixe du message (output)
     * arg command Commande du message (output)
     * return Vecteur des paramètres
     */
    std::vector<std::string> parseIRCMessage(const std::string& message, std::string& prefix, std::string& command)
    {
        // Vecteur des paramètres
        std::vector<std::string> params;

        // Si le message est vide, retourner un vecteur vide
        if (message.empty())
        {
            return params;
        }

        // Position actuelle dans le message
        size_t pos = 0;

        // Vérifier si le message commence par un préfixe
        if (message[0] == ':')
        {
            // Trouver la fin du préfixe
            size_t prefixEnd = message.find(' ');
            if (prefixEnd == std::string::npos)
            {
                // Message invalide
                return params;
            }

            // Extraire le préfixe
            prefix = message.substr(1, prefixEnd - 1);

            // Avancer la position
            pos = prefixEnd + 1;

            // Ignorer les espaces
            while (pos < message.size() && isspace(message[pos]))
            {
                pos++;
            }
        }

        // Extraire la commande
        size_t commandEnd = message.find(' ', pos);
        if (commandEnd == std::string::npos)
        {
            // Pas de paramètres
            command = message.substr(pos);
            return params;
        }

        // Extraire la commande
        command = message.substr(pos, commandEnd - pos);

        // Avancer la position
        pos = commandEnd + 1;

        // Ignorer les espaces
        while (pos < message.size() && isspace(message[pos]))
        {
            pos++;
        }

        // Extraire les paramètres
        while (pos < message.size())
        {
            // Si on trouve un caractère ':' au début d'un paramètre, tout le reste est un seul paramètre
            if (message[pos] == ':') {
                params.push_back(message.substr(pos + 1));
                break;
            }

            // Début du paramètre
            size_t start = pos;

            // Avancer jusqu'au prochain espace
            while (pos < message.size() && !isspace(message[pos]))
            {
                pos++;
            }

            // Ajouter le paramètre au vecteur
            params.push_back(message.substr(start, pos - start));

            // Ignorer les espaces
            while (pos < message.size() && isspace(message[pos]))
            {
                pos++;
            }
        }

        return params;
    }

    /**
     * Récupère le nom d'hôte de la machine
     * return Nom d'hôte
     */
    std::string getHostname()
    {
        // Buffer pour stocker le nom d'hôte
        char hostname[1024];
        hostname[1023] = '\0';

        // Récupérer le nom d'hôte
        gethostname(hostname, 1023);

        return std::string(hostname);
    }

    /**
     * Récupère l'adresse IP à partir d'un descripteur de fichier
     * arg fd Descripteur de fichier
     * return Adresse IP
     */
    std::string getIPFromFd(int fd)
    {
        // Structure pour stocker l'adresse
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);

        // Récupérer l'adresse
        if (getpeername(fd, (struct sockaddr*)&addr, &addrlen) < 0)
        {
            return "";
        }

        // Buffer pour stocker l'adresse IP
        char ipstr[INET6_ADDRSTRLEN];

        // Convertir l'adresse en chaîne
        if (addr.ss_family == AF_INET)
        {
            // IPv4
            struct sockaddr_in* s = (struct sockaddr_in*)&addr;
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
        }
        else
        {
            // IPv6
            struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
        }

        return std::string(ipstr);
    }

    /**
     * Génère une chaîne aléatoire
     * arg length Longueur de la chaîne
     * return Chaîne aléatoire
     */
    std::string generateRandomString(int length)
    {
        // Caractères possibles
        static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        // Initialiser le générateur de nombres aléatoires
        static bool initialized = false;
        if (!initialized)
        {
            srand(static_cast<unsigned int>(time(NULL)));
            initialized = true;
        }

        // Créer la chaîne
        std::string result;
        result.resize(length);

        // Remplir la chaîne avec des caractères aléatoires
        for (int i = 0; i < length; ++i)
        {
            result[i] = charset[rand() % (sizeof(charset) - 1)];
        }

        return result;
    }

    /**
     * Vérifie si un fichier existe
     * arg path Chemin du fichier
     * return true si le fichier existe, false sinon
     */
    bool fileExists(const std::string& path)
    {
        // Essayer d'ouvrir le fichier
        std::ifstream file(path.c_str());
        return file.good();
    }

    /**
     * Récupère l'extension d'un fichier
     * arg filename Nom du fichier
     * return Extension du fichier
     */
    std::string getFileExtension(const std::string& filename)
    {
        // Trouver le dernier point
        size_t dotPos = filename.find_last_of('.');

        // Si pas de point ou si c'est le premier caractère, pas d'extension
        if (dotPos == std::string::npos || dotPos == 0)
        {
            return "";
        }

        // Retourner l'extension
        return filename.substr(dotPos + 1);
    }

    /**
     * Récupère la taille d'un fichier
     * arg path Chemin du fichier
     * return Taille du fichier en octets
     */
    size_t getFileSize(const std::string& path)
    {
        // Structure pour stocker les informations du fichier
        struct stat st;

        // Récupérer les informations du fichier
        if (stat(path.c_str(), &st) < 0)
        {
            return 0;
        }

        return static_cast<size_t>(st.st_size);
    }

    /**
     * Sécurise une entrée utilisateur
     * arg input Entrée à sécuriser
     * return Entrée sécurisée
     */
    std::string sanitizeInput(const std::string& input)
    {
        // Créer une copie de l'entrée
        std::string result = input;

        // Remplacer les caractères potentiellement dangereux
        for (size_t i = 0; i < result.size(); ++i)
        {
            if (result[i] < 32 || result[i] > 126)
            {
                result[i] = '?';
            }
        }

        return result;
    }

}