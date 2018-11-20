#include "networked_player.h"

networked_player::networked_player(float maxPos, float minPos, sf::Vector2f startingPos, sf::Vector2f colliderSize, sf::Texture* playerTexture) : player(maxPos, minPos, startingPos, colliderSize, playerTexture)
{
	// initialise position message
	lastPositionUpdate = new PositionMessage;
	lastPositionUpdate->timeOfReception = 0.0f;
	lastPositionUpdate->position = startingPos;
	lastPositionUpdate->latestUpdateTime = 0.0f;

	secondLastPositionUpdate = new PositionMessage;
	secondLastPositionUpdate->timeOfReception = 0.0f;
	secondLastPositionUpdate->position = startingPos;
	lastPositionUpdate->latestUpdateTime = 0.0f;
	// init latency
	localLatency = 0.0f;
	// networked player will be handled differently - indiciate that this player is to be networked
	setNetworkedPlayer();
	// initialise elapsed time
	predictedTime = 0.0f;
}


networked_player::~networked_player()
{
	if(lastPositionUpdate)
	{
		delete lastPositionUpdate;
		lastPositionUpdate = nullptr;
	}
	if(secondLastPositionUpdate)
	{
		delete secondLastPositionUpdate;
		secondLastPositionUpdate = nullptr;
	}
	updateRecieved = false;
}

void networked_player::updateIncomingPosition(sf::Vector2f newPos, sf::Int64 newLookDir, float recvTime, float predictedTime, float sendTime)
{
	// if the newly recieved messsage's elasped time is more recent than the stored one...
	if(sendTime > lastPositionUpdate->latestUpdateTime)
	{
		// ..populate the position message struct
		secondLastPositionUpdate->position = lastPositionUpdate->position;
		secondLastPositionUpdate->timeOfReception = lastPositionUpdate->timeOfReception;
		lastPositionUpdate->position = newPos;
		lastPositionUpdate->timeOfReception = recvTime;
		lastPositionUpdate->latestUpdateTime = sendTime;
		// update player rotation
		updateObjRotation(static_cast<LOOK_DIR>(newLookDir));
		// confirm that we got an update and need to interpolate
		updateRecieved = true;
		this->predictedTime = predictedTime;
		// get the amount of latency experienced by this player
		localLatency = predictedTime - recvTime;
		// check if the player is moving
		const sf::Vector2f distVec = secondLastPositionUpdate->position - lastPositionUpdate->position;
		const float distance = sqrt(pow(distVec.x, 2) + pow(distVec.y, 2));
		if(distance > 0.005)
		{
			timer = 0.0f;
		}
	}
}

void networked_player::update(float& deltaTime)
{
	// increment elapsed time
	predictedTime += deltaTime;
	timer += deltaTime;
	// let's get the distance betwen the current position and the recieved position
	const sf::Vector2f distVec = getPosition() - lastPositionUpdate->position;
	const float distance = sqrt(pow(distVec.x, 2) + pow(distVec.y, 2));
	// check if the distance is small enough for us to translate without making prediction
	// predict the position for the next frame
	sf::Vector2f predictionVec = getPredictedPosition();
	sf::Vector2f organicPredict = getOrganicPredictedPosition(deltaTime);
	// get the interpolated position of the current position and the last update
	sf::Vector2f interpolatedPosition = interpolationAmnt * getPosition() + (1 - interpolationAmnt) * lastPositionUpdate->position;
	// assume the player is moving
	playWalkingAnim(deltaTime);
	// check for the time since the last update - if it was quite recent....
	if(timer < maxTimeDiff)
	{
		// check how big the latency is
		if(localLatency < predictLatency)
		{
			// if it is not too bad, make a distance check
			if(distance > minDistance)
			{
				if(updateRecieved)
				{
					// if it's larger than min distance, interpolate to that position if we recieved an update
					setPosition(interpolatedPosition);
					updateRecieved = false;
				}
				else
				{
					// just update using organic prediction if we didnt recieve an update
					setPosition(organicPredict);
				}
			}
			else
			{
				// otherwise, the player is probably not moving
				setPosition(lastPositionUpdate->position);
			}
		}
		else
		{
			// high latency - we will want to make position predictions. Check what prediction model we want to use
			const sf::Vector2f predDistVec = getPosition() - predictionVec;
			const float predDistance = sqrt(pow(distVec.x, 2) + pow(distVec.y, 2));
			if(predDistance > minPredDistance)
			{
				// interpolate between the predicted position and the current one
				sf::Vector2f predInterpolatedPosition = interpolationAmnt * getPosition() + (1 - interpolationAmnt) * predictionVec;
				// if the distance to our linear model is very high, set our player's position to it
				setPosition(predictionVec);
			}
			else
			{
				// otherwise, use organic movement prediciton
				sf::Vector2f newPos = getOrganicPredictedPosition(deltaTime);
				setPosition(newPos);
			}
		}
	}
	else
	{
		if(distance > minDistance)
		{
			setPosition(interpolatedPosition);
		}
		else
		{
			getSprite()->setTextureRect(getIdleRect());
			setPosition(lastPositionUpdate->position);
		}
	}

	//if(updateRecieved)
	//{
	//	//setPosition(lastPositionUpdate->position);
	//	updateRecieved = false;
	//}
	/*const sf::Vector2f predDistVec = getPosition() - predictionVec;
	const float predDistance = sqrt(pow(distVec.x, 2) + pow(distVec.y, 2));
	if (predDistance < minPredDistance)
	{
		predictionVec = getOrganicPredictedPosition(deltaTime);
	}
	setPosition(predictionVec);*/
}

sf::Vector2f networked_player::getPredictedPosition()
{
	sf::Vector2f predictedMovement;
	float avgVelocityX, avgVeloctiyY, time;
	// get elapsed time between the two known position updates
	time = (lastPositionUpdate->timeOfReception - secondLastPositionUpdate->timeOfReception);
	// calculate the average velocities for x and y
	avgVelocityX = (lastPositionUpdate->position.x - secondLastPositionUpdate->position.x) / time;
	avgVeloctiyY = (lastPositionUpdate->position.y - secondLastPositionUpdate->position.y) / time;
	// determine the predicted movement vector

	predictedMovement.x = avgVelocityX * (predictedTime - secondLastPositionUpdate->timeOfReception) + secondLastPositionUpdate->position.x;
	predictedMovement.y = avgVeloctiyY * (predictedTime - secondLastPositionUpdate->timeOfReception) + secondLastPositionUpdate->position.y;
	return (predictedMovement);
}

sf::Vector2f networked_player::getOrganicPredictedPosition(float& deltaTime)
{
	// find prediction based on the player's rotation
	const sf::Vector2f currentPosition = getPosition();
	sf::Vector2f movement = sf::Vector2f(0.0f, 0.0f);
	// iterate through the direction that the player is facing
	switch (getLookDir())
	{
	case LOOK_DIR::UP:
	{
		movement.y += (getMoveSpeed() * deltaTime);
		break;
	}
	case LOOK_DIR::RIGHT:
	{
		movement.x -= (getMoveSpeed() * deltaTime);
		break;
	}
	case LOOK_DIR::LEFT:
	{
		movement.x += (getMoveSpeed() * deltaTime);
		break;
	}
	case  LOOK_DIR::DOWN:
	{
		movement.y -= (getMoveSpeed() * deltaTime);
		break;
	}
	}
	// get the predicted position
	sf::Vector2f predictedMovement = (currentPosition - movement);
	// check that the predicted position is not out of the boundry of the level
	if (boundryCheck(predictedMovement))
	{
		predictedMovement = currentPosition;
	}
	return (predictedMovement);

}
