#include "projectile.h"



projectile::projectile(sf::Vector2f colliderSize, LOOK_DIR initDir, int firedPlayerID, sf::Texture* fireTexture, sf::Vector2f boundryInfo, sf::Vector2f initPosition) : game_object(colliderSize)
{
	// allocate projectile to a player
	ownerID = firedPlayerID;
	// setup the projectile's direction
	updateObjRotation(initDir);
	// setup the sprite of the player
	getSprite()->setTexture(*fireTexture);
	// setup min and max positions for the projectile, before the server deactivates them - also initialise the projectile's position
	obj_init(sf::Vector2f(boundryInfo.x, boundryInfo.x), sf::Vector2f(boundryInfo.y, boundryInfo.y), initPosition);
	active = true;
	// define nr of sprites on sheet
	const int spritesRow = 2;
	const int spritesCollumn = 1;
	// get size of sprite
	const sf::Vector2u spriteSize = sf::Vector2u(fireTexture->getSize().x / spritesRow, fireTexture->getSize().y / spritesCollumn); // tex_size.x /= sprite_nr;
	getSprite()->setTextureRect(sf::IntRect(0, 0, spriteSize.x, spriteSize.y));
	getSprite()->setOrigin(sf::Vector2f(static_cast<float>(spriteSize.x / 2), static_cast<float>(spriteSize.y / 2)));
	// initialise the animation
	getUpdateAnim()->init(fireTexture, sf::Vector2u(spritesRow, spritesCollumn), fireAnimTime);
}


projectile::~projectile()
{
}

void projectile::update(float& deltaTime)
{
	// find prediction based on the player's rotation
	const sf::Vector2f currentPosition = getPosition();
	sf::Vector2f movement = sf::Vector2f(0.0f, 0.0f);
	// iterate through the direction that the player is facing
	switch (getLookDir())
	{
	case LOOK_DIR::UP:
		{
		movement.y += (speed * deltaTime);
		break;
		}
	case LOOK_DIR::RIGHT:
		{
		movement.x -= (speed * deltaTime);
		break;
		}
	case LOOK_DIR::LEFT:
		{
		movement.x += (speed * deltaTime);
		break;
		}
	case  LOOK_DIR::DOWN:
		{
		movement.y -= (speed * deltaTime);
		break;
		}
	}
	// update animation
	getSprite()->setTextureRect(getUpdateAnim()->update(0, 0, deltaTime));
	// get the new position
	sf::Vector2f newPosition = (currentPosition - movement);
	setPosition(newPosition);
}
