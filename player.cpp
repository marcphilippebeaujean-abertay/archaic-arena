#include "player.h"

player::player(float maxPos, float minPos, sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* playerTexture) : game_object(colliderSize)
{
	//https://www.sfml-dev.org/tutorials/2.4/network-socket.php
	// init stats for player
	moveSpeed = initSpeed;
	setLookDir(LOOK_DIR::UP);
	// load texture
	getSprite()->setTexture(*playerTexture);
	// setup sprite sheet
	// get size of texture from sfml
	const sf::Vector2u texSize = playerTexture->getSize();
	// define nr of sprites on sheet
	const int spritesRow = 3;
	const int spritesCollumn = 1;
	// devide the height and width by the number of textures on the sheet to get the size of a single sprite on the sheet
	const sf::Vector2u spriteSize = sf::Vector2u(texSize.x / spritesRow, texSize.y / spritesCollumn); // tex_size.x /= sprite_nr;
	// create modifier to specify which state on the sprite sheet should be displayed
	const int spriteMod = 0;
	// first two params specify which sprite should be displayed, last two the exact size of the texture to be rendered
	idleRect = sf::IntRect(spriteSize.x * spriteMod, spriteSize.y * spriteMod, texSize.x / spritesRow, texSize.y / spritesCollumn);
	getSprite()->setTextureRect(idleRect);
	getSprite()->setOrigin(sf::Vector2f(static_cast<float>((spriteSize.x / 2)), static_cast<float>((spriteSize.y / 2))));
	// setup animation
	// create walking animation
	const float walkingAnimTime = 0.6f;
	getUpdateAnim()->init(playerTexture, sf::Vector2u(spritesRow, spritesCollumn), walkingAnimTime);
	// setup origin for rotation
	getSprite()->setOrigin(sf::Vector2f(static_cast<float>((spriteSize.x / 2)), static_cast<float>((spriteSize.y / 2))));
	// setup player position
	obj_init(sf::Vector2f(maxPos, maxPos), sf::Vector2f(minPos, minPos), startingPos);
	// store starting position
	startingPosition = startingPos;
	// base player will never be networked
	isNetworked = false;
	pendingUpdate = false;
}

player::~player()
{

}

void player::playWalkingAnim(const float& deltaTime)
{
	getSprite()->setTextureRect(getUpdateAnim()->update(0, 1, deltaTime));
}

void player::update(float& deltaTime)
{
	// determine where the player moves based on the direction they are facing
	sf::Vector2f movement(0.0f, 0.0f);

	// handle player input
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	{
		movement.y += (moveSpeed * deltaTime);
		updateObjRotation(LOOK_DIR::UP);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
		movement.x -= (moveSpeed * deltaTime);
		updateObjRotation(LOOK_DIR::RIGHT);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
	{
		movement.y -= (moveSpeed * deltaTime);
		updateObjRotation(LOOK_DIR::DOWN);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
	{
		movement.x += (moveSpeed * deltaTime);
		updateObjRotation(LOOK_DIR::LEFT);
	}

	if (movement.x == 0 && movement.y == 0)
	{
		// set character to idle state if they haven't moved
		getSprite()->setTextureRect(idleRect);
	}
	else
	{
		// update sprite animation and position
		playWalkingAnim(deltaTime);
		sf::Vector2f newPosition = getPosition();
		newPosition -= movement;
		// check that the player has not left the boundries of the game
		if(boundryCheck(newPosition) == false)
		{
			setPosition(newPosition);
		}
		// indicate that the player's position needs to be updated over the network
		pendingUpdate = true;
	}
}

void player::updateIncomingPosition(sf::Vector2f newPos, sf::Int64 newLookDir, float elapsedTime, float latency, float sendTime)
{
	
}

void player::resetPlayer()
{
	// reset the player to the state they were at the beginning of the game
	alive = true;
	setPosition(startingPosition);
	connected = true;
	moveSpeed = initSpeed;
}
