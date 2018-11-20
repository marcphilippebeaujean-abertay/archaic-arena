#pragma once
#include "transform.h"

class powerup : public transform
{
public:
	enum ItemType
	{
		speedup,
		speeddown,
		shotsup
	};

	// getters
	ItemType& getItemType() { return itemType; }
	bool& isActive() { return active; }

	// functions
	void resetPU();
	void setupPU(ItemType newItemType);

	powerup(sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* texture);
	~powerup();

private:
	bool active;
	// type of item that this object contains
	ItemType itemType;
	// sprite size, used to change sprite
	sf::IntRect speedUpRect;
	sf::IntRect speedDownRect;
	sf::IntRect shotsUpRect;
};

