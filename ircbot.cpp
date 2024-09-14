#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <cstdio>

#define NICK = "TestiBot"
#define SERVER = ("irc.paivola.fi",6667)
#define REALNAME = "Jehun botti"
#define DEFAULT_CHANNEL = "#botwars"
#define DEFAULT_CHANNEL_KEY = ""

using namespace std;

void answer(int clientSocket, bool *bot_running) {
	char buffer[1024] = { 0 };
	const char* message;
	
	while (*bot_running) { 
		recv(clientSocket, buffer, sizeof(buffer), 0);
		cout << buffer << endl;
		string newbuffer(buffer);
		if (newbuffer.substr(0,4) == "PING") {
			string msg = "PONG " + newbuffer.substr(5,9) + "\r\n";
			message = msg.c_str();
			send(clientSocket, message, strlen(message), 0);
		}
	}
}

int main() {
	bool bot_running = true;

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(6667);
	inet_pton(AF_INET, "135.181.107.23", &serverAddress.sin_addr);	
	connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	
	thread answer_thread(answer, clientSocket, &bot_running);
	
	const char* message = "NICK TestiBotti\r\n";
	send(clientSocket, message, strlen(message), 0);
	
	message = ":TestiBotti USER TestiBotti 0 * :Botti\r\n";
	send(clientSocket, message, strlen(message), 0);
	
	message = "MODE TestiBotti :+B\r\n";
	send(clientSocket, message, strlen(message), 0);
	
	string a;
	cin >> a;

	message = ":TestiBotti JOIN #botwars \r\n";
	send(clientSocket, message, strlen(message), 0);
	
	message = ":TestiBotti PRIVMSG #botwars :Mui.\r\n";
	send(clientSocket, message, strlen(message), 0);
	
	bot_running = false;
	answer_thread.join();
	close(clientSocket);
	return 0;
}
