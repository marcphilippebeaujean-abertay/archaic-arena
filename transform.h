#pragma once
#include <SFML/Graphics.hpp>

class transform
{
public:
	transform(sf::Vector2f colliderSize);
	~transform();

	// getters
	float& getX() { return position.x; }
	float& getY() { return position.y; }
	float& getRotation() { return zRot; }
	sf::Vector2f& getPosition() { return position; }
	sf::Sprite* getSprite() { return sprite; }
	sf::Vector2f getColliderSize() { return colliderSize; }

	// setters
	void setObjID(int id) { objID = id; }
	void setRotation(float zRot);
	void setPosition(sf::Vector2f& newPos);
	void setTransform(sf::Vector2f& newPos, float rot);
	void setCollider(sf::Vector2f colliderSize) { this->colliderSize = sf::Vector2f(colliderSize.x, colliderSize.y); }

	// functions
	bool isColliding(sf::Vector2f otherPosition, sf::Vector2f otherColliderSize);
	bool overlapping(float minA, float maxA, float minB, float maxB) { return minB <= maxA && minA <= maxB; }
	void drawCollider(sf::RenderWindow& window, sf::Shape& debugShape);

private:
	// struct that handles collision for our objects
	struct ColliderRect
	{
		// origin of the rectangle
		sf::Vector2f origin;
		// size of the rectangle
		sf::Vector2f size;
	};
	// transform information
	sf::Vector2f position;
	float zRot;
	int objID;
	sf::Vector2f colliderSize;
	// visual information
	sf::Sprite* sprite;
};

