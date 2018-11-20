#include "pillar.h"

pillar::pillar(sf::Vector2f position, sf::Texture* texture, int gridSize, sf::Vector2f colliderSize) : transform(colliderSize)
{
	// setup sprite
	getSprite()->setTextureRect(sf::IntRect(0, 0, gridSize, gridSize));
	getSprite()->setTexture(*texture);
	getSprite()->setOrigin(sf::Vector2f(static_cast<float>(gridSize / 2) - 2.0f, static_cast<float>(gridSize / 2) - 2.0f));
	setPosition(position);
}


pillar::~pillar()
{
}
