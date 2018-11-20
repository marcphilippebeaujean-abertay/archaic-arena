#include "game_object.h"

game_object::game_object(sf::Vector2f colliderSize) : transform(colliderSize)
{
	updateAnim = new animation();
}

game_object::~game_object()
{
	if (updateAnim)
	{
		delete updateAnim;
		updateAnim = nullptr;
	}
}

void game_object::obj_init(sf::Vector2f maxPos, sf::Vector2f minPos, sf::Vector2f initPos)
{
	this->maxPos = maxPos;
	this->minPos = minPos;
	setPosition(initPos);
}

void game_object::update(float& delta_time)
{

}

bool game_object::boundryCheck(sf::Vector2f& position)
{
	bool hitBoundry = false;
	if (position.x > getMaxPos().x)
	{
		hitBoundry = true;
	}
	if (position.x < getMinPos().x)
	{
		hitBoundry = true;
	}
	if (position.y > getMaxPos().y)
	{
		hitBoundry = true;
	}
	if (position.y < getMinPos().y)
	{
		hitBoundry = true;
	}
	return hitBoundry;
}

void game_object::updateObjRotation(LOOK_DIR new_dir)
{
	if (lookDir != new_dir)
	{
		lookDir = new_dir;
		// set the sprite to face in the right direction
		float new_rot = (90 * static_cast<float>(lookDir));
		setRotation(new_rot);
	}
}

