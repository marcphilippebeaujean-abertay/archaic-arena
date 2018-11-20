#include "powerup.h"

powerup::powerup(sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* texture) : transform(colliderSize)
{
	// load texture
	getSprite()->setTexture(*texture);
	// setup sprite sheet
	// get size of texture from sfml
	const sf::Vector2u texSize = texture->getSize();
	// define nr of sprites on sheet
	const int spritesRow = 3;
	const int spritesCollumn = 1;
	// devide the height and width by the number of textures on the sheet to get the size of a single sprite on the sheet
	const sf::Vector2u spriteSize = sf::Vector2u(static_cast<int>(texSize.x / spritesRow), (static_cast<int>(texSize.y / spritesCollumn))); // tex_size.x /= sprite_nr;
	// setup item rects
	speedUpRect = sf::IntRect(spriteSize.x * 0, spriteSize.y * 0, texSize.x / spritesRow, texSize.y / spritesCollumn);
	speedDownRect = sf::IntRect(spriteSize.x * 1, spriteSize.y * 0, texSize.x / spritesRow, texSize.y / spritesCollumn);
	shotsUpRect = sf::IntRect(spriteSize.x * 2, spriteSize.y * 0, texSize.x / spritesRow, texSize.y / spritesCollumn);
	// first two params specify which sprite should be displayed, last two the exact size of the texture to be rendered
	getSprite()->setTextureRect(speedDownRect);
	getSprite()->setOrigin(sf::Vector2f(static_cast<float>((spriteSize.x / 2)), static_cast<float>((spriteSize.y / 2))));
	setPosition(startingPos);
	// initialise the power up to being inactive
	active = false;
}


powerup::~powerup()
{
}

void powerup::resetPU()
{
	active = false;

}

void powerup::setupPU(ItemType newItemType)
{
	itemType = newItemType;

	// setup visuals for the item type
	switch (itemType)
	{
	case speedup:
		getSprite()->setTextureRect(speedUpRect);
		break;
	case speeddown:
		getSprite()->setTextureRect(speedDownRect);
		break;
	case shotsup:
		getSprite()->setTextureRect(shotsUpRect);
		break;
	}
	// set the power up to active
	active = true;
}
