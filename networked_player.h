#pragma once
#include "player.h"
#include <cmath>
class networked_player : public player
{
public:
	networked_player(float max_pos, float min_pos, sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* fileManager);
	~networked_player();

	// functions
	void updateIncomingPosition(sf::Vector2f newPos, sf::Int64 newLookDir, float recvTime, float elapsedTime, float sendTime) override;
	virtual void update(float& deltaTime) override;

	void setLatency(float latency) { localLatency = latency; }

private:
	// struct that contains the information we need to make updates on the networked player
	struct PositionMessage
	{
		float timeOfReception;
		float latestUpdateTime;
		sf::Vector2f position;
	};
	// vector that stores position and time information for each player update from the network
	PositionMessage* lastPositionUpdate;
	PositionMessage* secondLastPositionUpdate;
	// the amount we want to interpolate by - I have decided to always interpolate to the center point of our two vectors
	const float interpolationAmnt = 0.5f;
	// save our own elapsed time variable
	float predictedTime;
	// timer to handle update calls
	const float maxTimeDiff = 0.3f;
	float timer = 0.0f;
	// function to predict the player's position
	sf::Vector2f getPredictedPosition();
	sf::Vector2f getOrganicPredictedPosition(float& deltaTime);
	// check to see if we want to it interpolate during this frame
	bool updateRecieved = false;
	// if the distance between the last recieved position and the current one, we will set our position to the sent one
	const float minDistance = 0.5f;
	// latency of the networked player client
	float localLatency;
	// latency that needs to be exceeded before prediction is used
	const float predictLatency = 0.1f;
	// distance between predicted position and current position before organic prediction begins
	const float minPredDistance = 0.5f;
};