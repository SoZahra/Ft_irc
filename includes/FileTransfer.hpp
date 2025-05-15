#ifndef FILE_TRANSFER_HPP
# define FILE_TRANSFER_HPP

# include <string>       // Pour manipuler les chaînes de caractères
# include <map>          // Pour stocker les transferts en cours
# include <fstream>      // Pour la lecture/écriture des fichiers
# include <vector>       // Pour stocker les données de fichiers

# include "Client.hpp"

class Client;

// Structure pour stocker les détails d'un transfert de fichier
struct FileTransferInfo 
{
    Client*         sender;         // Client qui envoie le fichier
    Client*         receiver;       // Client qui reçoit le fichier
    std::string     filename;       // Nom du fichier en cours de transfert
    std::string     tempFilePath;   // Chemin du fichier temporaire
    size_t          fileSize;       // Taille totale du fichier
    size_t          bytesTransferred; // Nombre d'octets déjà transférés
    time_t          startTime;      // Heure de début du transfert
    std::ifstream   fileStream;     // Flux pour lire le fichier (envoi)
    std::ofstream   outputStream;   // Flux pour écrire le fichier (réception)
    bool            completed;      // Si le transfert est terminé
    std::string     transferId;     // Identifiant unique du transfert
};

class FileTransfer 
{
private:
    std::map<std::string, FileTransferInfo*> _transfers; // Transferts en cours
    std::string _tempDir;       // Répertoire pour les fichiers temporaires
    // size_t      _chunkSize;     // Taille des morceaux à envoyer à la fois
    
    // Méthodes privées
    std::string generateTransferId() const; // Génère un ID unique
    bool createTempDirectory(); // Crée le répertoire temporaire si nécessaire
    
public:
    // Constructeur et destructeur
    FileTransfer();
    ~FileTransfer();
    
    // Méthodes principales
    bool initTransfer(Client* sender, Client* receiver, const std::string& filename, size_t fileSize);
    bool sendFileChunk(const std::string& transferId);
    bool receiveFileChunk(const std::string& transferId, const std::vector<char>& data, size_t dataSize);
    bool cancelTransfer(const std::string& transferId);
    bool completeTransfer(const std::string& transferId);
    
    // Méthodes d'information
    FileTransferInfo* getTransferInfo(const std::string& transferId);
    std::vector<FileTransferInfo*> getTransfersForClient(Client* client);
    double getTransferProgress(const std::string& transferId);
    double getTransferSpeed(const std::string& transferId);
    
    // Méthodes utilitaires
    void cleanupCompletedTransfers();
};

#endif