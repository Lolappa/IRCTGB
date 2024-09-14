#include <cstring>
#include <iostream>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <cstdio>

#define NICK "TestiBotti"
#define SERVER = ("irc.paivola.fi",6667)
#define REALNAME = "Jehun botti"
#define DEFAULT_CHANNEL = "#botwars"
#define DEFAULT_CHANNEL_KEY = ""

using namespace std;

queue<string> send_queue = {};

vector<string> split_message(string command) {
	vector<string> tokens;
	for (int i = 0; i < 3; i++) {
		tokens.push_back(command.substr(0, command.find(" ")));
		command.erase(0, command.find(" ") + 1);
	}
	tokens.push_back(command);
	
	return tokens;
}

void send_message(string name, string channel, string message) {
	string out = "";
	out.append(":");
	out.append(NICK);
	out.append(" PRIVMSG ");
	out.append(channel);
	out.append(" :<");
	out.append(name);
	out.append("> ");
	out.append(message);
	out.append("\n");

	send_queue.push(out);
}

void send_process(int clientSocket, bool *bot_running) {
	while (*bot_running) {
		if (send_queue.size()) {
			string msg = send_queue.front();
			cout << msg << endl;
			const char* sendmsg = msg.c_str();
			send(clientSocket, sendmsg, strlen(sendmsg), 0);
			send_queue.pop();
		}
	}
}

void receive_process(int clientSocket, bool *bot_running) {
	char buffer[1024] = { 0 };
	string strbuffer = "";
	while (*bot_running) {
		recv(clientSocket, buffer, sizeof(buffer), 0);
		strbuffer = string(buffer);
		
		//extract the latest command
		string command = strbuffer.substr(0, strbuffer.find("\n"));
		cout << command << endl << "###############################################" << endl;
		
		// Check for a ping and respond
		if (command.find("PING") == 0) {
			string msg = "";
			msg.append("PONG ");
			msg.append(command.substr(command.find(":"), command.size() - 5));
			msg.append("\n");
			
			send_queue.push(msg);
		
		// Check if the command is a message
		} else if (split_message(command)[1] == "PRIVMSG") {
			vector<string> new_command = split_message(command);
			string name = new_command[0].substr(1, new_command[0].find("!") - 1);
			//find the second space in the command and extract the channel name
			string channel = new_command[2];
			string message = new_command[3].substr(1, new_command[3].size() - 1);
			send_message(name, channel, message);
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
	
	thread receiveProc(receive_process, clientSocket, &bot_running);
	thread sendProc(send_process, clientSocket, &bot_running);

	string message = "NICK TestiBotti\n";
	send_queue.push(message);

	message = ":TestiBotti USER TestiBotti 0 * :Botti\n";
	send_queue.push(message);
	
	message = "MODE TestiBotti :+B\n";
	send_queue.push(message);
	
	cout << "Mode Changed" << endl;

	string a;
	cin >> a;
	cout << endl;

	message = ":TestiBotti JOIN #botwars \n";
	send_queue.push(message);
	
	cout << "a" << endl;
	cin >> a;

	message = ":TestiBotti PRIVMSG #botwars :Mui.\n";
	send_queue.push(message);
	
	bot_running = false;
	receiveProc.join();
	sendProc.join();
	close(clientSocket);
	return 0;
}
