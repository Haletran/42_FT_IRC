#include "../includes/Global.hpp"
#include <csignal>
#include <fstream>

Bot::Bot(const std::string& server, int port, const std::string& nick, const std::string& chan, const std::string& password) : sockfd(-1), server_ip(server), port(port), nickname(nick),  channel(chan), password(password) {}

Bot::~Bot() {
    disconnect();
}

bool Bot::send_message(const std::string& message) {
    std::string msg = message + "\r\n";
    return send(sockfd, msg.c_str(), msg.size(), 0) >= 0;
}

bool Bot::connect_to_server() {
    struct sockaddr_in server_addr;
    struct hostent* host;

    host = gethostbyname(server_ip.c_str());
    if (host == NULL) {
        std::cerr << "Error: Unable to resolve host " << server_ip << std::endl;
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: Unable to create socket" << std::endl;
        return false;
    }

    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Unable to connect to server" << std::endl;
        return false;
    }

    return true;
}

void Bot::login() {
    send_message("PASS " + password);
    send_message("NICK " + nickname);
    send_message("USER " + nickname + " 0 * :" + nickname);
    sleep(1);
    send_message("JOIN " + channel);
    //send_message("TOPIC " + channel + " :🎮 Welcome to the game Channel :) !");
}

void Bot::printFile(std::string filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open script file" << std::endl;
        return;
    }
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        {
            lineCount++;
            if (!send_message("PRIVMSG " + channel + " :" + line)) {
                std::cerr << "Error: Unable to send message at line " << lineCount << std::endl;
                break;
            }
        }
        usleep(8000);
    }
}


void Bot::receive_messages() {
    char buffer[512];
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        std::string message(buffer);
        std::cout << message;

        if (message.find("!help") != std::string::npos)
            send_message("PRIVMSG " + channel + " :Here are the commands: !help, !ping, !ascii");
        else if (message.find("!ping") != std::string::npos)
            send_message("PRIVMSG " + channel + " :PONG");
        else if (message.find("!ascii") != std::string::npos)
        {
            switch(rand() % 3)
            {
                case 0:
                    printFile("includes/BotUtils/bapasqui");
                    break;
                case 1:
                    printFile("includes/BotUtils/tim");
                    break;
                case 2:
                    printFile("includes/BotUtils/Shrek");
                    break;
            }
        }
    }
}

void Bot::disconnect() {
    if (sockfd != -1) {
        send_message("PART " + channel + " :Leaving");
        close(sockfd);
        sockfd = -1;
    }
}

void Bot::SignalHandler(int signal) {
    (void)signal;
    throw std::runtime_error("Signal received");
}


int main() {
    srand(time(0));
    std::string server = "localhost";
    int port = 6666;
    std::string nickname = "BOT";
    std::string channel = "#game";
    std::string password = "testtest2";
    Bot client(server, port, nickname, channel, password);
    try {
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, client.SignalHandler);
        if (!client.connect_to_server()) {
            return 1;
        }
        client.login();
        client.receive_messages();
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        client.disconnect();
        return 1;
    }
    return 0;
}
