#include "file_manager.h"

file_manager::file_manager()
{
	player1Texture = new sf::Texture;
	// load texture
	if (!player1Texture->loadFromFile("../sprites/pl_1.png"))
	{
		// failed to load texture
		printf("failed to load texture!");
	}
	wallTexture = new sf::Texture;
	if (!wallTexture->loadFromFile("../sprites/brick_texture.png"))
	{
		// failed to load texture
	}
	else
	{
		wallTexture->setRepeated(true);
	}
	floorTexture = new sf::Texture;
	if (!floorTexture->loadFromFile("../sprites/floor_texture.png"))
	{
		// failed to load texture
	}
	else
	{
		floorTexture->setRepeated(true);
	}
	logo = new sf::Texture;

	menuFont = new sf::Font;
	if (!menuFont->loadFromFile("../fonts/sixty/sixty.ttf"))
	{
		// failed to load font!
	}

	logo = new sf::Texture;
	if (!logo->loadFromFile("../sprites/AAlogo.png"))
	{

	}

	player2Texture = new sf::Texture;
	if (!player2Texture->loadFromFile("../sprites/pl_2.png"))
	{
		// failed to load texture
	}

	projectileTexture = new sf::Texture;
	if (!projectileTexture->loadFromFile("../sprites/fireball.png"))
	{
		// failed to load texture
	}

	// create shape with radius
	debugShape = new sf::CircleShape(3);

	powerUpTex = new sf::Texture;
	if(!powerUpTex->loadFromFile("../sprites/poweruptex.png"))
	{
		
	}
}


file_manager::~file_manager()
{
	if (player1Texture)
	{
		delete player1Texture;
		player1Texture = nullptr;
	}
	if (wallTexture)
	{
		delete wallTexture;
		wallTexture = nullptr;
	}
	if (floorTexture)
	{
		delete floorTexture;
		floorTexture = nullptr;
	}
	if (logo)
	{
		delete logo;
		logo = nullptr;
	}
	if (menuFont)
	{
		delete menuFont;
		menuFont = nullptr;
	}
}
