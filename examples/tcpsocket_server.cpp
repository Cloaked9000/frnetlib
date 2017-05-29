#include <frnetlib/TcpSocket.h>
#include <frnetlib/TcpListener.h>

using namespace std;

int main(){
    //fr::HttpSocket<fr::TcpSocket> client; //fr::TcpSocket for HTTP. fr::SSLSocket for HTTPS.

	fr::TcpSocket client;
	

    fr::TcpListener listener;             //Use an fr::SSLListener if HTTPS.

	string port = "8081";

    //Bind to a port
    if(listener.listen(port) != fr::Socket::Success)
    {
		cout << "LISTENER:Failed to bind to port, going to shutdown" << endl;
		listener.shutdown();
		return -1;
    }

	cout << "LISTENER:Listener is listening on port " << port << " ..." << endl;

	while (true)
	{
		cout << "LISTENER:Waiting for a new connection ..." << endl;
		//Accept a new connection
		if (listener.accept(client) != fr::Socket::Success)
		{
			cout << "LISTENER:Failed to accept client, shutdown" << endl;
			break;
		}

		while (true) // infinate loop for the communication
		{
			try
			{
				fr::Packet packet;
				if (client.receive(packet) != fr::Socket::Success)
				{
					cout << "LISTENER:Failed to receive request" << endl;
				}

				std::string str1, str2;
				float age;
				packet >> str1 >> age >> str2;

				cout << "LISTENER:We got from client:" << str1 << age << str2 << endl;


				if (client.send(packet) != fr::Socket::Success)
				{
					cout << "LISTENER:Seems got something wrong when sending" << endl;
					//return -2;
				}
			}
			catch (const std::exception& e)
			{
				cout << "ERROR: " << e.what() << endl;
				cout << "LISTENER:Seems that the client stop the connection, just destory current connection and wait for another" << endl;
				//Close connection
				client.close_socket();
				break;
			}
		} // inner while


	}// out while

	cout << "Should not got there !!" << endl;
	listener.shutdown();
	return 0;
}

