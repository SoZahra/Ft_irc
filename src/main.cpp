#include <iostream>       // Pour les entrées/sorties standards
#include <cstdlib>        // Pour les fonctions standards comme atoi
#include <csignal>        // Pour la gestion des signaux
#include <string>         // Pour les chaînes de caractères
#include "../includes/Server.hpp"     // Notre classe serveur IRC
#include "../includes/Utils.hpp"      // Fonctions utilitaires

// Variables globales
Server* g_server = NULL;  // Pointeur global vers le serveur pour pouvoir l'arrêter proprement
bool g_running = true;    // Variable d'état pour contrôler la boucle principale

/**
 * Gestionnaire de signal pour arrêter proprement le serveur
 * arg signal Le signal reçu (e.g., SIGINT)
 */
void signalHandler(int signal)
{
    // Log du signal reçu
    Utils::logMessage("Signal reçu: " + Utils::toString(signal));

    // Arrêter le serveur si possible
    if (g_server)
    {
        g_server->stop();
    }

    // Marquer le serveur comme devant s'arrêter
    g_running = false;
}

/**
 * Configure les gestionnaires de signaux
 */
void setupSignalHandlers()
{
    // Configurer le gestionnaire pour SIGINT (Ctrl+C)
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        Utils::logMessage("Impossible de configurer le gestionnaire pour SIGINT", true);
    }

    // Configurer le gestionnaire pour SIGTERM
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        Utils::logMessage("Impossible de configurer le gestionnaire pour SIGTERM", true);
    }
}

/**
 * Affiche l'usage correct du programme
 */
void showUsage(const std::string& programName)
{
    std::cout << "Usage: " << programName << " <port> <password>" << std::endl;
    std::cout << "  <port>     : Le port sur lequel le serveur écoute (1024-65535)" << std::endl;
    std::cout << "  <password> : Le mot de passe pour se connecter au serveur" << std::endl;
}

/**
 * Valide les arguments du programme
 * arg port Le port fourni
 * @return true si le port est valide, false sinon
 */
bool validatePort(int port)
{
    // Vérifier que le port est dans une plage valide (au-dessus des ports privilégiés)
    return port > 1023 && port < 65536;
}

/**
 * Fonction principale
 */
int main(int argc, char* argv[])
{
    // Vérifier le nombre d'arguments
    if (argc != 3)
    {
        std::cerr << "Erreur: nombre d'arguments incorrect." << std::endl;
        showUsage(argv[0]);
        return 1;
    }

    // Récupérer les arguments
    int port = atoi(argv[1]);
    std::string password = argv[2];

    // Valider le port
    if (!validatePort(port))
    {
        std::cerr << "Erreur: port invalide. Le port doit être entre 1024 et 65535." << std::endl;
        return 1;
    }

    // Configurer les gestionnaires de signaux
    setupSignalHandlers();

    try
    {
        // Créer et démarrer le serveur
        g_server = new Server(port, password);

        // Afficher un message de démarrage
        std::cout << "Démarrage du serveur IRC sur le port " << port << std::endl;

        // Démarrer le serveur
        g_server->start();

        // Le serveur tourne dans un thread séparé, attendre qu'il s'arrête
        while (g_running)
        {
            // Pause pour éviter de consommer trop de CPU
            usleep(100000); // 100ms
        }

        // Nettoyage
        delete g_server;
        g_server = NULL;

        std::cout << "Serveur arrêté proprement." << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        // En cas d'erreur, afficher le message et nettoyer
        std::cerr << "Erreur fatale: " << e.what() << std::endl;

        if (g_server)
        {
            delete g_server;
            g_server = NULL;
        }

        return 1;
    }
}