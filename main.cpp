#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "game_manager.h"
#include "connection_manager.h"
#include "menu_manager.h"
#include "sfml_client.h"
#include "sfml_server.h"
#include "file_manager.h"

enum GAMESTATE
{
	menu,
	loading,
	playing,
	endcredits
};

int main()
{
	bool debuggingGame = false;

	// create game state
	GAMESTATE myState;
	if (debuggingGame)
	{
		myState = playing;
	}
	else
	{
		myState = menu;
	}

	// INITIALISE MANAGERS
	// create file manager, mainly used to store textures in memory
	file_manager* fileManager = new file_manager;
	// create manager for the main game loop
	game_manager* gameManager = new game_manager(fileManager);
	// create manager to handle the menu
	menu_manager* menuManager = new menu_manager(gameManager->getView()->getCenter(), fileManager);

	// INITIALISE NETWORK DEPENDANCIES
	// define port
	const unsigned short tcpPort = 4444;
	// define IP address
	const sf::IpAddress IPaddr = "127.0.0.1";
	// define time before sockets unblock
	const sf::Time blockingTimeval = sf::milliseconds(30);
	// create manager for our sfml socket network
	connection_manager* networkManager = nullptr;

	// setup delta time
	float deltaTime = 0.0f;
	sf::Clock clock;

	// check if window is open...
    while (gameManager->getWindow()->isOpen())
	{
		// WINDOW EVENTS
		// create event to handle our window
		sf::Event m_windowEvnt;
		while (gameManager->getWindow()->pollEvent(m_windowEvnt))
		{
			switch (m_windowEvnt.type)
			{
			case sf::Event::Closed:
				// triggered when player closes window, closes console (and therefore the program)
				gameManager->getWindow()->close();
				break;
			case sf::Event::Resized:
				// triggered when player reshapes window, prints new window params to console
				gameManager->resizeView();
				menuManager->adjustTextPosition(gameManager->getView()->getCenter());
				break;
			case sf::Event::TextEntered:
				// triggered when player inputs keyboard input
				break;
			case sf::Event::KeyPressed:
				// if we are in the menu....
				if (myState == menu)
				{
					// ...iterate again to navigate the menu
					switch (m_windowEvnt.key.code)
					{
					case sf::Keyboard::W:
					case sf::Keyboard::Up:
						menuManager->updateSelection(true);
						break;
					case sf::Keyboard::Down:
					case sf::Keyboard::S:
						menuManager->updateSelection(false);
						break;
					case sf::Keyboard::Left:
					case sf::Keyboard::A:
						if(menuManager->getCurSelectedItem() == 0)
						{
							menuManager->updatePlayerNr(true);
						}
						break;
					case sf::Keyboard::Right:
					case sf::Keyboard::D:
						if (menuManager->getCurSelectedItem() == 0)
						{
							menuManager->updatePlayerNr(false);
						}
						break;
					case sf::Keyboard::Space:
						// user wants to interact with the selected item!
						switch (menuManager->getCurSelectedItem())
						{
						case 0: // host match
							myState = loading;
							menuManager->toggleLoadingScreen();
							// clear the connection manager pointer so we can assign it to be a server
							delete networkManager;
							networkManager = new sfml_server(IPaddr, tcpPort, blockingTimeval, menuManager->getHostNr()); // menuManager->getHostNr()
							// we need to set our connection manager reference after we have initialised our server manager, otherwise it gets deleted
							gameManager->setConnectionManager(networkManager);
							break;
						case 1: // find match
							myState = loading;
							menuManager->toggleLoadingScreen();
							networkManager = new sfml_client(IPaddr, tcpPort, blockingTimeval);
							// we need to set our connection manager reference after we have initialised our server manager, otherwise it gets deleted
							gameManager->setConnectionManager(networkManager);
							break;
						case 2: // quit
							gameManager->getWindow()->close();
							break;
						default:
							gameManager->getWindow()->close();
							break;
						}
						break;
					default:
						break;
					}
				}
				// controls that are available at other states
				switch (m_windowEvnt.key.code)
				{
				case sf::Keyboard::Q:
					gameManager->getWindow()->close();
					break;
				case sf::Keyboard::Space:
					if(myState == endcredits)
					{
						myState = menu;
						menuManager->toggleMain();
					}
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		// UPDATE EVENTS
		// determine game behaviour based on state
		switch (myState)
		{
		case menu:
			menuManager->drawMenu(*gameManager->getWindow());
			break;
		case loading:
			if (networkManager->isServer())
			{
				// display the number of connections made by the server in the menu
				menuManager->toggleHostPlayerCount(networkManager->getConnectionNr());
			}
			else
			{
				if(networkManager->getConnectionNr() > 0)
				{
					menuManager->toggleClientConnectionStatus(true);
				}
				else
				{
					menuManager->toggleClientConnectionStatus(false);
				}
			}
			menuManager->drawMenu(*gameManager->getWindow());
			// this boolean will return true if the server confirms that all players have connected
			if (networkManager->socketsConnected())
			{
				myState = playing;
			}
			else
			{
				// we are still waiting on the confirmation message
			}
			networkManager->update(deltaTime);
			break;
		case playing:
			// returns the time elasped (delta time) in seconds
			deltaTime = clock.restart().asSeconds();
			networkManager->update(deltaTime);
			if (gameManager->isGameOver() == false)
			{
				// update game
				gameManager->update(deltaTime);
			}
			else
			{
				if (networkManager->isServer() == false)
				{
					// if we weren't acting as the server, simply close all connections
					menuManager->toggleFinalScreen(gameManager->getScoreList(), gameManager->getID());
					// load the endcredits
					myState = endcredits;
					// delete the network manager
					if (networkManager)
					{	
						networkManager->closeConnection();
						networkManager = nullptr;
					}
					break;
				}
				else
				{
					// if we were functioning as the server, close the program
					gameManager->getWindow()->close();
					break;
				}
			}
			break;
		case endcredits:
			menuManager->drawMenu(*gameManager->getWindow());
			// display the end credits
			break;
		}
	}
	// clear up pointers
	if (gameManager)
	{
		delete gameManager;
		gameManager = nullptr;
	}
	if (menuManager)
	{
		delete menuManager;
		menuManager = nullptr;
	}
	if (fileManager)
	{
		delete fileManager;
		fileManager = nullptr;
	}
	return 0;
}
