#pragma once
#include "game_object.h"
class projectile : public game_object
{
public:
	projectile(sf::Vector2f colliderSize, LOOK_DIR initDir, int firedPlayerID, sf::Texture* fireTexture, sf::Vector2f boundryInfo, sf::Vector2f initPosition);
	~projectile();

	// functions
	void update(float& delta_time) override;

	// getters
	bool isActive() { return active; }
	int getOwner() { return ownerID; }

	// setters
	void deactivate() { active = false; }
private:
	// speed that the projectile moves
	const float speed = 100.0f;
	// variable that indicates which player the projectile should ignore
	int ownerID;
	// variable used to determine whether the projectile should be rendered and updated
	bool active;
	// rate at which the fire animation should be played
	const float fireAnimTime = 0.4f;
};

