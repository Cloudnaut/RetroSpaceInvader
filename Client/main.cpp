//////////////////////////////////////////////////
//Retro Multiplayer Invader Client
//////////////////////////////////////////////////

#include <SFML\System.hpp>
#include <SFML\Network.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include "FileSystem.hpp"
#define CLIENT_VERSION 1

enum State
{
	Connecting,
	Waiting,
	Playing,
	Closing,
	NotAccepted,
};

class InformationBundle
{
public:
	State *m_DependingState;
	sf::Vector2f *m_PlayerPosition;
	sf::Vector2f *m_OpponentPosition;
	InformationBundle(State *DependingState, sf::Vector2f *PlayerPosition, sf::Vector2f *OpponentPosition)
	{
		m_DependingState = DependingState;
		m_PlayerPosition = PlayerPosition;
		m_OpponentPosition = OpponentPosition;
	}
};

void ReceiveAndSend(InformationBundle *Bundle)
{
	bool GameStarted = false;
	sf::TcpSocket Server;

	while(true)
	{
		if(*Bundle->m_DependingState == State::Connecting)
		{
			sf::IpAddress ServerIP = sf::IpAddress::LocalHost; // Hier muss der Servername hin

			sf::Socket::Status Status;

			Status = Server.connect(ServerIP, 666);

			while(Status != sf::Socket::Done)
			{
				Status = Server.connect(ServerIP, 666);
			}

			sf::Packet DataPacket;
			DataPacket << CLIENT_VERSION;
			sf::Socket::Status SendStatus = Server.send(DataPacket);

			while(Status != sf::Socket::Done)
			{
				SendStatus = Server.send(DataPacket);
			}

			bool ConnectionAccepted = false;
			DataPacket.clear();
			Server.receive(DataPacket);
			DataPacket >> ConnectionAccepted;

			if(ConnectionAccepted == true)
				*Bundle->m_DependingState = State::Waiting;
			else
				*Bundle->m_DependingState = State::NotAccepted;
		}
		else if(*Bundle->m_DependingState == State::Waiting)
		{
			sf::Packet DataPacket;

			if(GameStarted)
			{
				*Bundle->m_DependingState = State::Playing;
			}
			else////////////////////////////////////////////////////////// <-- Eventuell in Thread auslagern wegen anzeigeproblemen
			{
				DataPacket.clear();
				Server.receive(DataPacket);
				DataPacket >> GameStarted;
			}
		}
		else if(*Bundle->m_DependingState == State::Playing) //Spielerpostionen, Schutz und Bullets Updaten
		{
			//Eigene Spielerposition senden

			sf::Packet DataPacket;
			DataPacket << Bundle->m_PlayerPosition->x;
			Server.send(DataPacket);

			//Daten vom Server bekommen

			DataPacket.clear();
			Server.receive(DataPacket);
			DataPacket >> Bundle->m_OpponentPosition->x;

		}
	}
}

int main()
{
	sf::RenderWindow *Window = new sf::RenderWindow(sf::VideoMode(800, 600),  "Retro Multiplayer Invader");
	State CurrentState = State::Connecting;

	sf::Font StandardFont;

	if(!StandardFont.loadFromFile(akilib::System::FileSystem::GetExecDirectory() + "AGENCYR.ttf"))
	{
		exit(0xDEAD);
	}

	sf::Text StateTxt;
	StateTxt.setColor(sf::Color::White);
	StateTxt.setPosition(50, Window->getSize().y - 100);
	StateTxt.setFont(StandardFont);

	sf::Vector2f PlayerPosition = sf::Vector2f(Window->getSize().x / 2, Window->getSize().y - 50);
	sf::Vector2f OpponentPosition = sf::Vector2f(Window->getSize().x / 2, 50);

	sf::CircleShape PlayerShape(20, 3);
	PlayerShape.setOrigin(20, 20);
	PlayerShape.setFillColor(sf::Color::Blue);

	sf::CircleShape OpponentShape(20, 3);
	OpponentShape.setOrigin(20, 20);
	OpponentShape.setFillColor(sf::Color::Green);
	OpponentShape.setRotation(180);

	InformationBundle *InfoBundle = new InformationBundle(&CurrentState, &PlayerPosition, &OpponentPosition); 

	sf::Thread *ReceiveAndSendThread = new sf::Thread(&ReceiveAndSend, InfoBundle);
	ReceiveAndSendThread->launch();

	sf::Event Event;

	while(Window->isOpen())
	{
		Window->clear();
		///////////////////Eingabe auswerten//////////////////////////

		Window->pollEvent(Event);

		PlayerShape.setPosition(PlayerPosition);
		OpponentShape.setPosition(OpponentPosition);

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			PlayerPosition.x -= 0.25;
		}
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			PlayerPosition.x += 0.25;
		}

		///////////////////Anzeigende Tätigkeiten/////////////////////

		//CurrentState = State::Playing; //Temporär für anzeigefunkionen

		if(CurrentState == State::Connecting)
		{
			StateTxt.setString("Establishing connection...");
			Window->draw(StateTxt);
		}

		if(CurrentState == State::Waiting)
		{
			StateTxt.setString("Waiting for open game...");
			Window->draw(StateTxt);
		}

		if(CurrentState == State::Playing)
		{
			Window->draw(PlayerShape);
			Window->draw(OpponentShape);
		}

		if(CurrentState == State::NotAccepted)
		{
			StateTxt.setString("Connection not accepted.");
			Window->draw(StateTxt);
		}

		//////////////////////////////////////////////////////////////
		Window->display();
		///////////////////Verarbeitende Tätigkeiten//////////////////

		
		

		//////////////////////////////////////////////////////////////
	}
}