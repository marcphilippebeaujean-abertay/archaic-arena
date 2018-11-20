#include "transform.h"

transform::transform(sf::Vector2f colliderSize)
{
	sprite = new sf::Sprite;
	setCollider(colliderSize);
}


transform::~transform()
{
	if (sprite)
	{
		delete sprite;
		sprite = nullptr;
	}
}

void transform::setTransform(sf::Vector2f& new_pos, float z_rot)
{
	setPosition(new_pos);
	this->zRot = z_rot;
}

void transform::setPosition(sf::Vector2f& newPos)
{
	if (sprite->getTexture())
	{
		position = newPos;
		sprite->setPosition(newPos);
	}
}

void transform::setRotation(float z_rot)
{
	this->zRot = z_rot;
	sprite->setRotation(z_rot);
}

bool transform::isColliding(sf::Vector2f otherPosition, sf::Vector2f otherColliderSize)
{
	// generate the collider dimenions for this transform
	const float thisLeft = (position.x - colliderSize.x);
	const float thisRight = (position.x + colliderSize.x);
	const float thisBottom = (position.y - colliderSize.y);
	const float thisTop = (position.y + colliderSize.y);
	const sf::Vector2f thisTopRight = sf::Vector2f(position.x + colliderSize.x, position.y + colliderSize.y);
	// generate collider dimensions for the other transform
	const float otherLeft = (otherPosition.x - otherColliderSize.x);
	const float otherRight = (otherPosition.x + otherColliderSize.x);
	const float otherBottom = (otherPosition.y - otherColliderSize.y);
	const float otherTop = (otherPosition.y + otherColliderSize.y);
	// finally, check for overlap
	return overlapping(thisLeft, thisRight, otherLeft, otherRight) && overlapping(thisBottom, thisTop, otherBottom, otherTop);
}

void transform::drawCollider(sf::RenderWindow& window, sf::Shape& debugShape)
{
	// create vector of points
	std::vector<sf::Vector2f> colliderPoints;
	// generate collider points
	const sf::Vector2f bottomLeft = sf::Vector2f(position.x - colliderSize.x, position.y - colliderSize.y);
	colliderPoints.push_back(bottomLeft);
	const sf::Vector2f topLeft = sf::Vector2f(position.x - colliderSize.x, position.y + colliderSize.y);
	colliderPoints.push_back(topLeft);
	const sf::Vector2f bottomRight = sf::Vector2f(position.x + colliderSize.x, position.y - colliderSize.y);
	colliderPoints.push_back(bottomRight);
	const sf::Vector2f topRight = sf::Vector2f(position.x + colliderSize.x, position.y + colliderSize.y);
	colliderPoints.push_back(topRight);
	// draw each point
	for(auto i = 0; i < colliderPoints.size(); i++)
	{
		debugShape.setPosition(colliderPoints[i]);
		window.draw(debugShape);
	}
}

