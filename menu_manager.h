#pragma once
#include "SFML\Graphics.hpp"
#include "file_manager.h"
#include <sstream>

#define MAX_ITEM_PER_MENU 8
#define CHAR_SIZE 30

class menu_manager
{
public:
	menu_manager(sf::Vector2f windowDimensions, file_manager* fileManager);
	~menu_manager();

	// navigation
	void updateSelection(const bool moveUp);
	void drawMenu(sf::RenderWindow &window);
	void adjustTextPosition(const sf::Vector2f viewDimensions);
	void toggleMain();
	void toggleLoadingScreen();
	void updatePlayerNr(bool subtract);
	void toggleFinalScreen(std::vector<int> playerScores, int& playerID);
	void toggleHostPlayerCount(int& playerCount);
	void toggleClientConnectionStatus(bool connected);

	// getters
	int& getCurSelectedItem() { return curButton; };
	int& getHostNr() { return curPlayerHostNr; }

private:
	int curButton;
	int curMaxItem;

	sf::Text* menuText[MAX_ITEM_PER_MENU];
	sf::Sprite* logo;

	int curPlayerHostNr = 2;

	void resetColor();
};

