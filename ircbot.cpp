#include <cstring>
#include <iostream>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <cstdio>
#include <fstream>

#define NICK "TestiBotti"
#define SERVER = ("irc.paivola.fi",6667)
#define REALNAME = "Jehun botti"
#define DEFAULT_CHANNEL = "#botwars"
#define DEFAULT_CHANNEL_KEY = ""

using namespace std;

queue<string> bot_send_queue = {};
queue<string> irc_send_queue = {};

string fifo_file_name = "./bridge";
string service_name = "IRC";
fstream fifo_file;

vector<string> bot_split_message(string message) {
	vector<string> tokens = {};
	while (message.size() > 0) {
		tokens.push_back(message.substr(0, message.find("\n")));
		message.erase(0, message.find("\n") + 1);
	}
	return tokens;
}

vector<string> split_message(string command) {
	vector<string> tokens = {};
	for (int i = 0; i < 3; i++) {
		tokens.push_back(command.substr(0, command.find(" ")));
		command.erase(0, command.find(" ") + 1);
	}
	tokens.push_back(command);
	
	return tokens;
}

void irc_send_message(string name, string channel, string service, string message) {
	string out = "";
	out.append(":");
	out.append(NICK);
	out.append(" PRIVMSG ");
	out.append(channel);
	out.append(" :[");
	out.append(service);
	out.append("]<");
	out.append(name);
	out.append("> ");
	out.append(message);
	out.append("\n");

	irc_send_queue.push(out);
}

void bot_send_message(string name, string channel, string message) {
	string out = "";
	out.append(name);
	out.append("\n");
	out.append(channel);
	out.append("\n");
	out.append(service_name);
	out.append("\n");
	out.append(message);
	out.append("\n\n");
	
	bot_send_queue.push(out);
}

void fifo_write(string name, string channel, string message) {
	fifo_file << name << endl << channel << endl << service_name << endl << message << endl;
}

void bot_send_process(int botSocket, bool *bot_running) {
	while (*bot_running) {
		if (bot_send_queue.size()) {
			string msg = bot_send_queue.front();
			const char* sendmsg = msg.c_str();
			//cout << send(botSocket, sendmsg, strlen(sendmsg), 0) << endl;
			bot_send_queue.pop();
		}
	}
}

void irc_send_process(int clientSocket, bool *bot_running) {
	while (*bot_running) {
		if (irc_send_queue.size()) {
			string msg = irc_send_queue.front();
			cout << msg << endl;
			const char* sendmsg = msg.c_str();
			send(clientSocket, sendmsg, strlen(sendmsg), 0);
			irc_send_queue.pop();
		}
	}
}

void bot_receive_process(int botSocket, bool *bot_running) {
	if (fifo_file.is_open()) {
		while (!fifo_file.eof()) {
			string name;
			string channel;
			string service;
			getline(fifo_file, name);
			getline(fifo_file, channel);
			getline(fifo_file, service);
			
			if (service != service_name) {
				string line = "";
				vector<string> lines = {};
				getline(fifo_file, line);
				while (line[0] != (char)30) {
					lines.push_back(line);
					getline(fifo_file, line);
				}
				
				for (string message: lines) {
					cout << name << endl << channel << endl << service << endl << message << endl;
					irc_send_message(name, channel, service, message);
				}
			}
		}
	}
	/*char buffer[1024] = { 0 };
	string strbuffer = "";
	while (*bot_running) {
		int buffer_length = recv(botSocket, buffer, sizeof(buffer), 0);
		if (buffer_length > 1) {
			int pos = 0;
			strbuffer = string(buffer);
			while (strbuffer.find("\n", pos) != buffer_length - 1) {
				string name = strbuffer.substr(0, strbuffer.find("\n"));
				pos = strbuffer.find("\n") + 1;
				string channel = strbuffer.substr(pos, strbuffer.find("\n")-pos);
				pos = strbuffer.find("\n") + 1;
				string service = strbuffer.substr(pos, strbuffer.find("\n")-pos);
				pos = strbuffer.find("\n") + 1;
				string message = strbuffer.substr(pos, strbuffer.find("\n\n")-pos-1);
				
				vector<string> lines = bot_split_message(message);
				if (service != service_name) {
					for (string i: lines) {
						cout << name << endl << channel << endl << service << endl << message << endl;
						irc_send_message(name, channel, service, message);
					}
				}
			}
		}
	}*/
}

void irc_receive_process(int clientSocket, bool *bot_running) {
	char buffer[1024] = { 0 };
	string strbuffer = "";
	while (*bot_running) {
		if (recv(clientSocket, buffer, sizeof(buffer), 0) > 0) {
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
				
				irc_send_queue.push(msg);
			
			// Check if the command is a message
			} else if (split_message(command)[1] == "PRIVMSG") {
				vector<string> new_command = split_message(command);
				string name = new_command[0].substr(1, new_command[0].find("!") - 1);
				//find the second space in the command and extract the channel name
				string channel = new_command[2];
				string message = new_command[3].substr(1, new_command[3].size() - 1);
				irc_send_message(name, channel, service_name, message);
				fifo_write(name, channel, message);
				//bot_send_message(name, channel, message);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	{ // Process arguments
			for (int i = 1; i < argc; i++) {
			string argument = string(argv[i]);
			if (argument.find("--filename=") == 0) {
				fifo_file_name = argument.substr(11, argument.size()-11);
			} else
			if (argument.find("--name=") == 0) {
				service_name = argument.substr(7, argument.size()-7);
			}
		}
	}
	fifo_file.open(fifo_file_name);
	
	bool bot_running = true;
	
	int botSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in botAddress;
	botAddress.sin_family = AF_INET;
	botAddress.sin_port = htons(8080);
	botAddress.sin_addr.s_addr = INADDR_ANY;
	cout << connect(botSocket, (struct sockaddr*)&botAddress, sizeof(botAddress)) << endl << strerror(errno) << endl;

	// Connect to pvlnet
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(6667);
	inet_pton(AF_INET, "135.181.107.23", &serverAddress.sin_addr);	
	cout << connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) << endl << strerror(errno) << endl;
	
	// Create Threads
	thread botReceiveProc(bot_receive_process, botSocket, &bot_running);
	thread ircReceiveProc(irc_receive_process, clientSocket, &bot_running);
	//thread botSendProc(bot_send_process, botSocket, &bot_running);
	thread ircSendProc(irc_send_process, clientSocket, &bot_running);
	
	string message = "NICK TestiBotti\n";
	irc_send_queue.push(message);
	
	message = ":TestiBotti USER TestiBotti 0 * :Botti\n";
	irc_send_queue.push(message);
	
	message = "MODE TestiBotti :+B\n";
	irc_send_queue.push(message);
	
	cout << "Mode Changed" << endl;
	
	string a;
	cin >> a;
	cout << endl;
	
	message = ":TestiBotti JOIN #botwars \n";
	irc_send_queue.push(message);
	
	cout << "a" << endl;
	cin >> a;
	
	message = ":TestiBotti PRIVMSG #botwars :Mui.\n";
	irc_send_queue.push(message);
	
	cout << "Exiting" << endl;
	bot_running = false;
	botReceiveProc.join();
	ircReceiveProc.join();
	//botSendProc.join();
	ircSendProc.join();
	close(clientSocket);
	return 0;
}
