#pragma once
#include <SFML\Graphics.hpp>

class file_manager
{
public:
	file_manager();
	~file_manager();

	sf::Texture* getPlTex() { return player1Texture; }
	sf::Texture* getNetPlTex() { return player2Texture; }
	sf::Texture* getWallTex() { return wallTexture; }
	sf::Texture* getFloorTex() { return floorTexture; }
	sf::Texture* getLogo() { return logo; }
	sf::Texture* getProjectileTex() { return projectileTexture; }
	sf::Texture* getPowerUpTex() { return powerUpTex; }

	sf::Shape* getShape() { return debugShape; }

	sf::Font* getMainMenuFont() { return menuFont; }
private:
	sf::Texture* player1Texture;
	sf::Texture* player2Texture;
	sf::Texture* wallTexture;
	sf::Texture* floorTexture;
	sf::Texture* projectileTexture;
	sf::Texture* logo;
	sf::Texture* powerUpTex;

	sf::Shape* debugShape;

	sf::Font* menuFont;
};

