#include "menu_manager.h"

menu_manager::menu_manager(sf::Vector2f windowDimensions, file_manager* fileManager)
{
	// load logo and texture
	logo = new sf::Sprite;
	// apply texture to logo sprite
	logo->setTexture(*fileManager->getLogo());
	// setup texture rect
	logo->setTextureRect(sf::IntRect(0, 0, fileManager->getLogo()->getSize().x, fileManager->getLogo()->getSize().y));

	// initialise font, position, size, etc. for menu text
	for (auto i = 0; i < MAX_ITEM_PER_MENU; i++)
	{
		menuText[i] = new sf::Text;
		menuText[i]->setFont(*fileManager->getMainMenuFont());
		menuText[i]->setCharacterSize(CHAR_SIZE);
		menuText[i]->setFillColor(sf::Color::White);
		menuText[i]->setStyle(sf::Text::Bold);
	}
	// allign text to the center of the window
	adjustTextPosition(windowDimensions);
	// setup main menu
	toggleMain();
}

menu_manager::~menu_manager()
{
	for (auto i = 0; i < MAX_ITEM_PER_MENU; i++)
	{
		delete menuText[i];
		menuText[i] = nullptr;
	}
}

void menu_manager::updateSelection(const bool moveUp)
{
	// check if user moved up a menu slot and see if we need to hard-set their selection
	if(moveUp)
	{
		curButton--;
		if(curButton < 0)
		{
			curButton = (curMaxItem - 1);
		}
	}
	else
	{
		curButton++;
		if(curButton == curMaxItem)
		{
			curButton = 0;
		}
	}
	// reset the color of all items and set the color of the menu item that is now selected
	resetColor();
}

void menu_manager::toggleMain()
{
	// reset cur button
	curButton = 0;
	// the  main menu has three items in it
	curMaxItem = 3;
	// set the first item to be red by default when the player enters the main menu
	resetColor();
	// create stream to handle hosting option
	std::ostringstream hostDisp;
	hostDisp << "Host Match       Players: " << curPlayerHostNr;
	// set menu text to display the wanted text
	menuText[0]->setString(hostDisp.str());
	menuText[1]->setString("Find Match");
	menuText[2]->setString("Quit");

	for(auto i = 3; i < MAX_ITEM_PER_MENU; i++)
	{
		// reset the text of every string that's not on the main menu
		menuText[i]->setString("");
	}
}

void menu_manager::toggleLoadingScreen()
{
	resetColor();
	// we only need one string to display the loading screen and no button navigation
	menuText[0]->setString("Waiting for all players to connect!");
	menuText[0]->setFillColor(sf::Color::White);
	menuText[1]->setString("");
	menuText[2]->setString("");
}

void menu_manager::toggleHostPlayerCount(int& playerCount)
{
	std::ostringstream hostDisp;
	hostDisp << "Number of Connected Players: " << playerCount;
	menuText[0]->setString(hostDisp.str());
}

void menu_manager::toggleClientConnectionStatus(bool connected)
{
	if(connected)
	{
		menuText[0]->setString("Connected! Waiting for more players...");
	}
	else
	{
		menuText[0]->setString("Attempting to connect to game...");
	}
}


void menu_manager::toggleFinalScreen(std::vector<int> playerScores, int& playerID)
{
	// iterate through the player scores
	for (auto i = 0; i < playerScores.size(); i++)
	{
		// check if this player is the local player
		if(i == playerID)
		{
			// if they are, set the colour to white
			menuText[i]->setFillColor(sf::Color::Green);
		}
		else
		{
			menuText[i]->setFillColor(sf::Color::White);
		}
		// create stream to handle player scores as a string
		std::ostringstream scoreString;
		scoreString << "Player " << i+1 << " : " << playerScores[i];
		// set it in the 
		menuText[i]->setString(scoreString.str());
	}
	// give the player the option to return to the menu
	menuText[playerScores.size() + 1]->setFillColor(sf::Color::Red);
	menuText[playerScores.size() + 1]->setString("Press space to return to menu");
}


void menu_manager::resetColor()
{
	for(auto i = 0; i < MAX_ITEM_PER_MENU; i++)
	{
		if(i == curButton)
		{
			menuText[i]->setFillColor(sf::Color::Red);
		}
		else
		{
			menuText[i]->setFillColor(sf::Color::White);
		}
	}
}

void menu_manager::drawMenu(sf::RenderWindow &window)
{
	// clear buffer
	window.clear();
	// draw menu text
	for(auto i = 0; i < MAX_ITEM_PER_MENU; i++)
	{
		window.draw(*menuText[i]);
	}
	// draw logo
	window.draw(*logo);
	// display elements
	window.display();
}

void menu_manager::adjustTextPosition(const sf::Vector2f viewDimensions)
{
	const float heightPadding = 3.0f;
	const float heightPos = 2.0f;
	const float logoPadding = 0.2f;
	const float withdPadding = 6.0f;
	for (auto i = 0; i < MAX_ITEM_PER_MENU; i++)
	{
		menuText[i]->setPosition((viewDimensions.x / withdPadding), (viewDimensions.y / heightPadding)  * (i + heightPos));
	}
	logo->setPosition((viewDimensions.x / withdPadding), (viewDimensions.y / 2) * (logoPadding));
}

void menu_manager::updatePlayerNr(bool subtract)
{
	if(subtract)
	{
		curPlayerHostNr--;
	}
	else
	{
		curPlayerHostNr++;
	}
	if(curPlayerHostNr < 2)
	{
		curPlayerHostNr = 4;
	}
	if(curPlayerHostNr > 4)
	{
		curPlayerHostNr = 2;
	}
	toggleMain();
}
