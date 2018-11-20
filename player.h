#pragma once
#include <SFML/Graphics.hpp>
#include "game_object.h"

class player : public game_object
{
public:
	player(float maxPos, float minPos, sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* playerTexture);
	~player();

	// getters
	float& getMoveSpeed() { return moveSpeed; }
	bool& getNetworkedPlayer() { return isNetworked; }
	bool& getPendingUpdate() { return pendingUpdate; }
	sf::IntRect& getIdleRect() { return idleRect; }
	bool& isAlive() { return alive; }
	int& getScore() { return score; }
	bool& isConnected() { return connected; }

	// setters
	void setMoveSpeed(float new_speed) { moveSpeed = new_speed; }
	void setNetworkedPlayer() { isNetworked = true; }
	void resetPendingUpdate() { pendingUpdate = false; }
	void killPlayer() { alive = false; }
	void setScore(int newScore) { score = newScore; }
	void resetPlayer();
	void disconnectPlayer() { connected = false; }
	void setPendingUpdate() { pendingUpdate = true; }
	void setSlowSpeed() { moveSpeed = slowSpeed; }
	void setFastSpeed() { moveSpeed = fastSpeed; }

	virtual void update(float& deltaTime) override;
	virtual void updateIncomingPosition(sf::Vector2f newPos, sf::Int64 newLookDir, float recvTime, float elapsedTime, float sendTime);
	void playWalkingAnim(const float& deltaTime);

private:
	// speed at which the character moves
	float moveSpeed;
	const float initSpeed = 80.0f;
	const float slowSpeed = 70.0f;
	const float fastSpeed = 90.0f;
	sf::IntRect idleRect;
	// ID used by the server to determine which player to move
	bool isNetworked;
	// boolean used by the game manager to see if updates to the player need to be made
	bool pendingUpdate;
	// check if player is alive
	bool alive = false;
	// score of the player
	int score = 0;
	// vector that stores the position that the player started at
	sf::Vector2f startingPosition;
	// variable that checks if the player with this id is still connected
	bool connected = false;
};

