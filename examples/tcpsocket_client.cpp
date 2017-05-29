#include <frnetlib/TcpSocket.h>
#include <frnetlib/TcpListener.h>

using namespace std;



int client_round(fr::TcpSocket& socket)
{
	cout << "CLIENT:Going to send something ..." << endl;

	//Receive the request
	fr::Packet packet;
	packet << "Hello there, I am" << (float)1.2 << "years old";
	if (socket.send(packet) != fr::Socket::Success)
	{
		//Failed to send packet
		cout << "CLIENT:Seems got something wrong when sending" << endl;
		return -1;
	}

	cout << "CLIENT:Going to receive ..." << endl;


	if (socket.receive(packet) != fr::Socket::Success)
	{
		cout << "CLIENT:seems got something wrong when receiving" << endl;
		return -2;
	}

	std::string str1, str2;
	float age;
	packet >> str1 >> age >> str2;

	cout << "CLIENT:we got:" << str1 << age << str2 << endl;

	cout << "CLIENT:round finished" << endl;
	cout << endl << endl << endl << endl;
	return 0;
}


int main()
{
	fr::TcpSocket socket;
	if (socket.connect("127.0.0.1", "8081") != fr::Socket::Success)
	{
		//Failed to connect
		cout << "CLIENT:it seem that the socket can be accessed or there is no such socket at all" << endl;
		socket.close_socket();
		return -1;
	}


	string op_str;
	int rtn = 0;

	while (true) {
		cout << "CLIENT:choose what you want to do, `c` for `continue`, `q` for `quit`:" << endl;
		cin >> op_str; // count for possible mutiple char input

		if (op_str.length() > 1) {
			cout << "CLIENT:Seems that you input more than one char, plese check your input" << endl;
			continue;
		}

		char op = op_str[0];
		switch (op) {
		case 'c':
			cout << "continue" << endl;
			rtn = client_round(socket);
			break;
		case 'q':
			break;
		}

		if (op == 'q')
			break;
		if (rtn != 0)
			break;
	}

	socket.close_socket();
	cout << "all done, bye" << endl;
}

