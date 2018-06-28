//////////////////////////////////////////////////
//Retro Multiplayer Invader Server
//////////////////////////////////////////////////

#include <SFML\System.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <list>
#include <iomanip>
using namespace std;
#define SERVER_VERSION 1

class Client
{
private:
	int m_Version;
	bool m_GameStarted;
	float m_XPosition;
	float m_OpponentXPosition;
	sf::TcpSocket *m_ClientSocket;
public:
	void SetGameStarted(bool Value)
	{
		m_GameStarted = Value;
	}
	float GetPlayerXPosition()
	{
		return m_XPosition;
	}
	void SetOpponentXPosition(float Position)
	{
		m_OpponentXPosition = Position;
	}
	sf::TcpSocket *GetClientSocket()
	{
		return m_ClientSocket;
	}
	void SetClientSocket(sf::TcpSocket *Socket)
	{
		m_ClientSocket = Socket;
	}
	bool IsValid()
	{
		if(!m_ClientSocket)
			return false;
		else return true;
	}
	void Update()
	{
		sf::Packet DataPacket;

		m_ClientSocket->receive(DataPacket);

		DataPacket >> m_Version;

		if(m_Version != SERVER_VERSION)
		{
			cout << "WARNING for Client " << m_ClientSocket->getRemoteAddress() << ": Different versions." << endl;
		}

		DataPacket.clear();
		DataPacket << true; //True für Angenommen, false für abgelehnt
		m_ClientSocket->send(DataPacket);

		bool InitalPhase = true;

		while(true)
		{
			DataPacket.clear();

			if(InitalPhase)
			{
				if(m_GameStarted == true)
				{
					DataPacket << true;

					sf::Socket::Status SendStatus = m_ClientSocket->send(DataPacket);

					while(SendStatus != sf::Socket::Done)
					{
						SendStatus = m_ClientSocket->send(DataPacket);
					}

					InitalPhase = false;
				}
			}
			else //Spielerposition und alles an den Client übermitteln
			{
				//Informationen von dem Client erhalten

				m_ClientSocket->receive(DataPacket);
				DataPacket >> m_XPosition;

				//Informationen an den Client senden

				DataPacket.clear();
				DataPacket << m_OpponentXPosition;
				m_ClientSocket->send(DataPacket);
			}

			

			sf::sleep(sf::milliseconds(100));
		}
	}
	Client()
	{
		m_Version = 0;
		m_GameStarted = false;
		m_ClientSocket = NULL;
	}
};

class Couple
{
private:
	Client *m_Client01;
	Client *m_Client02;
public:
	Client *GetClient01()
	{
		return m_Client01;
	}
	Client *GetClient02()
	{
		return m_Client02;
	}
	void SetClient01(Client *NewClient)
	{
		m_Client01 = NewClient;
	}
	void SetClient02(Client *NewClient)
	{
		m_Client02 = NewClient;
	}
	bool IsComplete()
	{
		if(!m_Client01 || !m_Client02)
			return false;
		else return true;
	}
	Couple()
	{
		m_Client01 = NULL;
		m_Client02 = NULL;
	}
};

class Session
{
private:
	int m_ID;
	Couple *m_ClientCouple;
public:
	int GetID()
	{
		return m_ID;
	}


	void SetID(int Value)
	{
		m_ID = Value;
	}


	Couple *GetCouple()
	{
		return m_ClientCouple;
	}


	void SetCouple(Couple *NewCouple)
	{
		m_ClientCouple = NewCouple;
	}


	bool IsComplete()
	{
		if(m_ClientCouple)
		{
			if(m_ClientCouple->IsComplete())
				return true;
		}
		else return false;
	}


	void Update()
	{
		bool InitalPhase = true;

		while(true)
		{
			if(m_ClientCouple->IsComplete() && InitalPhase)
			{
				m_ClientCouple->GetClient01()->SetGameStarted(true);
				m_ClientCouple->GetClient02()->SetGameStarted(true);
				InitalPhase = false;
			}
			
			if(!InitalPhase)
			{
				m_ClientCouple->GetClient01()->SetOpponentXPosition(m_ClientCouple->GetClient02()->GetPlayerXPosition());
				m_ClientCouple->GetClient02()->SetOpponentXPosition(m_ClientCouple->GetClient01()->GetPlayerXPosition());
			}

			sf::sleep(sf::milliseconds(100));
		}
	}


	Session()
	{
		m_ID = NULL;
		m_ClientCouple = NULL;
	}
};


int main()
{
	std::list<Session *> Sessions;
	sf::TcpListener Listener;
	Listener.listen(666);

	Session *CurrentSession = new Session;
	sf::Thread *CurrentSessionThread = new sf::Thread(&Session::Update, CurrentSession);
	CurrentSessionThread->launch();

	while(true)
	{
		if(!CurrentSession->IsComplete()) //Wenn die Session nicht fertig ist
		{
			if(CurrentSession->GetCouple()) //Couple existiert bereits
			{
				sf::TcpSocket *CurrentClientSocket = new sf::TcpSocket;

				sf::TcpListener::Status Status = Listener.accept(*CurrentClientSocket);

				while(Status != sf::Socket::Done)
				{
					Status = Listener.accept(*CurrentClientSocket);
				}

				Client *CurrentClient = new Client;
				
				CurrentClient->SetClientSocket(CurrentClientSocket);

				cout << setfill('0') << "New client connected: " << CurrentClient->GetClientSocket()->getRemoteAddress() << " -> Session " << setw(4) << CurrentSession->GetID();

				if(!CurrentSession->GetCouple()->GetClient01())
				{
					CurrentSession->GetCouple()->SetClient01(CurrentClient);
					cout << " -> Client01" << endl;
				}
				else
				{
					CurrentSession->GetCouple()->SetClient02(CurrentClient);
					cout << " -> Client02" << endl;
				}

				sf::Thread *Thread = new sf::Thread(&Client::Update, CurrentClient);
				Thread->launch();
			}
			else //Couple existiert nicht
			{
				Couple *CurrentCouple = new Couple;
				CurrentSession->SetCouple(CurrentCouple);
				CurrentSessionThread = new sf::Thread(&Session::Update, CurrentSession);
				CurrentSessionThread->launch();
			}
		}
		else //Wenn die Session fertig ist
		{
			Sessions.push_back(CurrentSession);
			CurrentSession = new Session;
			CurrentSession->SetID(Sessions.back()->GetID() + 1);
			
		}
		sf::sleep(sf::milliseconds(2500));
	}

}