#include "game_manager.h"

game_manager::game_manager(file_manager* fileManager)
{
	// create the view, window and level when the game starts

	// determine size of view
	viewSize = (levelSize + gridSize);

	// create window
	window = new sf::RenderWindow(sf::VideoMode(viewSize, viewSize), "SFML works!", sf::Style::Close | sf::Style::Resize);

	// create view
	view = new sf::View(sf::Vector2f(static_cast<float>(viewSize / 2), static_cast<float>(viewSize / 2)), sf::Vector2f(static_cast<float>(viewSize), static_cast<float>(viewSize)));

	// create wall that indicates the level boundries
	sf::RectangleShape* wall = new sf::RectangleShape(sf::Vector2f(static_cast<float>(viewSize), static_cast<float>(viewSize)));
	// setup wall textures
	// setup texture and bind to the rectangle object
	wall->setTexture(fileManager->getWallTex());
	wall->setTextureRect(sf::IntRect(0, 0, viewSize, viewSize));
	environmentAssets.push_back(wall);

	// create floor of arena
	sf::RectangleShape* floor = new sf::RectangleShape(sf::Vector2f((static_cast<float>(levelSize - gridSize)), (static_cast<float>(levelSize - gridSize))));
	floor->setPosition(sf::Vector2f(static_cast<float>(gridSize), static_cast<float>(gridSize)));
	// setup texture and bind to the rectangle object
	floor->setTexture(fileManager->getFloorTex());
	floor->setTextureRect(sf::IntRect(0, 0, levelSize, levelSize));
	environmentAssets.push_back(floor);

	// create pillars
	instantiatePillars(fileManager->getWallTex());

	// initialise power up spawns
	instantiatePowerUpSpawns(fileManager->getPowerUpTex());
	resetPowerUps();
	requirePuRespawn = true;

	// setup debug shape
	debugShape = fileManager->getShape();

	// setup player texture
	playerTexture = fileManager->getPlTex();

	// setup fire texture
	fireTex = fileManager->getProjectileTex();

	// setup networked player texture
	enemyTexture = fileManager->getNetPlTex();

	// initialise players
	for (auto i = 0; i < 4; i++)
	{
		player* newPlayer = new networked_player(static_cast<float>(levelSize), static_cast<float>(1 + gridSize), sf::Vector2f(0, 0), plColliderSize, enemyTexture);
		newPlayer->killPlayer();
		playerList.push_back(newPlayer);
	}

	// initialise elapsed time
	elapsedTime = 0.0f;

	// set connection manager to nullptr
	connectionManager = nullptr;

	// setup timers
	fireTimer = 0.0f;
	fireRate = initFireRate;
	pingPongTimer = 0.0f;

	latencyUpdateRequired = true;
}


game_manager::~game_manager()
{
	// clean up pointers
	for (auto i = 0; i < playerList.size(); i++)
	{
		delete playerList[i];
		playerList[i] = nullptr;
	}
	playerList.clear();
	if (window)
	{
		delete window;
		window = nullptr;
	}
	for (auto i = 0; i < environmentAssets.size(); i++)
	{
		delete environmentAssets[i];
		environmentAssets[i] = nullptr;
	}
	environmentAssets.clear();
	for(auto i = 0; i < brickPillars.size(); i++)
	{
		delete brickPillars[i];
		brickPillars[i] = nullptr;
	}
	brickPillars.clear();
	environmentAssets.clear();
	if (view)
	{
		delete view;
		view = nullptr;
	}
	if (debugShape)
	{
		delete debugShape;
		debugShape = nullptr;
	}
	if (connectionManager)
	{
		delete connectionManager;
		connectionManager = nullptr;
	}
	for (auto i = 0; i < powerUps.size(); i++)
	{
		delete powerUps[i];
		powerUps[i] = nullptr;
	}
	clearProjectiles();
}

void game_manager::clearProjectiles()
{
	for (auto i = 0; i < projectiles.size(); i++)
	{
		delete projectiles[i];
		projectiles[i] = nullptr;
	}
	projectiles.clear();
}


void game_manager::update(float& deltaTime)
{
	// update all objects in the world that are not static
	// call update on each player
	for (auto i = 0; i < totalPlayers; i++)
	{
		// check if the player is alive, otherwise we don't need to update them
		if (playerList[i]->isAlive())
		{
			// store the player's position at the beginning of the frame
			sf::Vector2f plInitPos = playerList[i]->getPosition();
			playerList[i]->update(deltaTime);
			// check if the local player has collided with anything
			for (auto i = 0; i < brickPillars.size(); i++)
			{
				if (playerList[localID]->isColliding(brickPillars[i]->getPosition(), brickPillars[i]->getColliderSize()))
				{
					// if they have, reset their position
					playerList[localID]->setPosition(plInitPos);
				}
			}
		}
	}
	// update projectiles
	for(auto i = 0; i < projectiles.size(); i++)
	{
		if(projectiles[i]->isActive())
		{
			projectiles[i]->update(deltaTime);
		}
	}
	// update the total time that our game has been running for
	elapsedTime += deltaTime;
	if(connectionManager->isServer() == false)
	{
		// increment the latency timer
		latencyTimer += deltaTime;
	}
	// check if we are the server
	if (connectionManager->isServer() == false)
	{
		// check if we waited long enough to fire a projectile
		if (fireTimer > fireRate)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
			{
				// check that our player is alive
				if (playerList[localID]->isAlive())
				{
					// tell the server that we want to instantiate a projectile with TCP
					message initProjectile;
					// when sending, incorporate our own latency (the time we expect it will take for the server to recieve our message and send it onwards)
					initProjectile.initPacketValues(2, playerList[localID]->getPosition(), playerList[localID]->getLookDirNr(), localID, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
					connectionManager->addToSendQueue(initProjectile);
					fireTimer = 0.0f;
				}
			}
		}
		else
		{
			// otherwise, update the fire timer
			fireTimer += deltaTime;
		}
	}
	else
	{
		// if we are acting as the server, we are responsible for spawning power ups
		if(requirePuRespawn)
		{
			// increment our timer
			powerUpTimer += deltaTime;
			// once reached, we need to setup a power up
			if(powerUpTimer > powerUpRespawnTime)
			{
				// indicate that a power up has spawned - we don't want more than one power up in the game at once
				requirePuRespawn = false;
				// generate both item type and item position randomly
				int randSelection = thor::random(0, 2);
				int randItem = thor::random(0, 2);
				// create a message containing the information we just 
				message puMsg;
				puMsg.initPacketValues(5, sf::Vector2f(0, 0), randItem, randSelection, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
				// add the message to the outoing message queue
				connectionManager->addToSendQueue(puMsg);
				// initialise the power up on the server
				powerUps[randSelection]->setupPU(static_cast<powerup::ItemType>(randItem));

			}
		}
		else
		{
			// we know that there is an active power up
			for(auto i = 0; i < powerUps.size(); i++)
			{
				if(powerUps[i]->isActive())
				{
					// check for each player if they have collected the power up
					for(auto x = 0; x < totalPlayers; x++)
					{
						if(playerList[x]->isAlive())
						{
							if (powerUps[i]->isColliding(playerList[x]->getPosition(), playerList[x]->getColliderSize()))
							{
								// create the message and add it to the queue
								message collectMsg;
								collectMsg.initPacketValues(6, sf::Vector2f(0, 0), i, x, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
								// we want to add one message to send out and one for the server's game manager to handle
								connectionManager->addToSendQueue(collectMsg);
								connectionManager->addToRcvQueue(collectMsg);
								powerUpTimer = 0.0f;
								requirePuRespawn = true;
							}
						}
					}
				}
			}
		}
		// check for each particle
		for (auto i = 0; i < projectiles.size(); i++)
		{
			// check if the projectile we are checking is active
			if (projectiles[i]->isActive())
			{
				bool hasCollided = false;
				if(projectiles[i]->boundryCheck(projectiles[i]->getPosition()))
				{
					message projCollMessage;
					projCollMessage.initPacketValues(3, sf::Vector2f(0, 0), static_cast<sf::Int16>(i), 0, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
					connectionManager->addToSendQueue(projCollMessage);
					deactivateProjectile(i);
					hasCollided = true;
				}
				if (hasCollided == false)
				{
					for (auto z = 0; z < brickPillars.size(); z++)
					{
						// check if our projectile has collided with a pillar
						if (projectiles[i]->isColliding(brickPillars[z]->getPosition(), brickPillars[z]->getColliderSize()))
						{
							// create message
							message projCollMessage;
							// it has - we should destroy it
							projCollMessage.initPacketValues(3, sf::Vector2f(0, 0), static_cast<sf::Int16>(i), 0, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
							connectionManager->addToSendQueue(projCollMessage);
							deactivateProjectile(i);
							hasCollided = true;
							break;
						}
					}
				}
				if (hasCollided == false)
				{
					// check collision for each player
					for (auto x = 0; x < totalPlayers; x++)
					{
						// the projectile should ignore the person who shot it and make sure the player we are checking for is alive
						if (projectiles[i]->getOwner() != x && playerList[x]->isAlive())
						{
							if (projectiles[i]->isColliding(playerList[x]->getPosition(), playerList[x]->getColliderSize()))
							{
								// player has collided with a particle - we should kill them!
								// create message
								message projCollMessage;
								// it has - we should destroy it
								projCollMessage.initPacketValues(4, sf::Vector2f(0, 0), static_cast<sf::Int64>(i), static_cast<sf::Int16>(x), elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
								connectionManager->addToSendQueue(projCollMessage);
								connectionManager->addToRcvQueue(projCollMessage);
								break;
							}
						}
					}
				}
			}
			else
			{

			}
		}
	}

	// check if the local player's position has been updated
	if (connectionManager->isServer() == false)
	{
		// we want to control the speed at which we send messages, so as not to overload the socket
		if ((elapsedTime - lastCallTime) > networkUpdateRate)
		{
			// if so, send a network message...
			if (connectionManager)
			{
				lastCallTime = elapsedTime;
				message positionUpdateMessage;
				// send the position update combined with the local latency
				positionUpdateMessage.initPacketValues(1, playerList[localID]->getPosition(), playerList[localID]->getLookDirNr(), localID, curLatency, message::MESSAGE_TYPE::UDP, elapsedTime);
				printf("sent a message with id %d", localID);
				connectionManager->addToSendQueue(positionUpdateMessage);
				// ...and reset the pending update flag
				playerList[localID]->resetPendingUpdate();
			}
		}
	}
	if(connectionManager->isServer() == false)
	{
		// check for projectile collision
	}
	// check the network manager's event queue
	handleNetworkMessages();
	// send out a ping pong message if the game has started
	if(connectionManager->socketsConnected())
	{
		// and if the timer has been reached
		if(pingPongTimer > pingPongRate)
		{
			// check that our last update request has returned
			if (latencyUpdateRequired)
			{
				message pingPongMessage;
				// we don't need to fill it with anything except that we want to send it with tcp - we just want to reset the timer
				pingPongMessage.initPacketValues(7, sf::Vector2f(0, 0), 0, localID, elapsedTime, message::MESSAGE_TYPE::TCP, elapsedTime);
				connectionManager->addToSendQueue(pingPongMessage);
				// reset timers for latency and ping pong
				pingPongTimer = 0.0f;
				latencyTimer = 0.0f;
				latencyUpdateRequired = false;
			}
		}
		else
		{
			pingPongTimer += deltaTime;
		}
	}
	// render game world
	render();
}

void game_manager::render()
{
	// clear buffer
	window->clear();
	// update view
	window->setView(*view);
	debugShape->setFillColor(sf::Color::Blue);
	// render objects to buffer
	for(auto i = 0; i < environmentAssets.size(); i++)
	{
		window->draw(*environmentAssets[i]);
	}
	// draw each player
	for(auto i = 0; i < playerList.size(); i++)
	{
		if(playerList[i]->isAlive())
		{
			window->draw(*playerList[i]->getSprite());
			//playerList[i]->drawCollider(*window, *debugShape);
		}
	}
	// draw pillars
	for(auto i = 0; i < brickPillars.size(); i++)
	{
		window->draw(*brickPillars[i]->getSprite());
		//brickPillars[i]->drawCollider(*window, *debugShape);
;	}
	// render projectiles
	for(auto i = 0; i < projectiles.size(); i++)
	{
		if(projectiles[i]->isActive())
		{
			window->draw(*projectiles[i]->getSprite());
			//projectiles[i]->drawCollider(*window, *debugShape);
		}
	}
	for(auto i = 0; i < powerUps.size(); i++)
	{
		debugShape->setFillColor(sf::Color::Red);
		if (powerUps[i]->isActive())
		{
			window->draw(*powerUps[i]->getSprite());
			// powerUps[i]->drawCollider(*window, *debugShape);
		}
	}
	// set buffer to window and display objects
	window->display();
}

void game_manager::setupPlayers(const int playerID, const sf::Int64 amountOfPlayers)
{
	// setup the ID for the local player
	localID = playerID;
	// set different starting positions for each player
	sf::Vector2f plStartingPosition;
	for (auto i = 0; i < amountOfPlayers; i++)
	{
		// setup the position for our new player
		switch (i)
		{
			case 0:
			{
				// bottom right
				plStartingPosition = sf::Vector2f(static_cast<float>(levelSize - 1), static_cast<float>(levelSize - 1));
				break;
			}
			case 1:
			{
				// top left
				plStartingPosition = sf::Vector2f(static_cast<float>(1 + gridSize), static_cast<float>(1 + gridSize));
				break;
			}
			case 2:
			{
				// top right
				plStartingPosition = sf::Vector2f(static_cast<float>(1 + gridSize), static_cast<float>(levelSize - 1));
				break;
			}
			case 3:
			{
				// bottom left
				plStartingPosition = sf::Vector2f(static_cast<float>(levelSize - 1), static_cast<float>(1 + gridSize));
				break;
			}
			default:
			{
				// bottom right
				plStartingPosition = sf::Vector2f(static_cast<float>(levelSize - 1), static_cast<float>(levelSize - 1));
				break;
			}
		}
		// see which player is our local player and which one is networked players
		if(i == playerID && connectionManager->isServer() == false)
		{
			// initiate local player if it corresponds to the ID and we aren't acting as the server
			delete playerList[i];
			playerList[i] = new player(static_cast<float>(levelSize), static_cast<float>(1 + gridSize), plStartingPosition, plColliderSize, playerTexture);
		}
		else
		{
			delete playerList[i];
			playerList[i] = new networked_player(static_cast<float>(levelSize), static_cast<float>(1 + gridSize), plStartingPosition, plColliderSize, enemyTexture);
		}
		// set this player to alive, as they are in game
		playerList[i]->resetPlayer();
		playersAlive++;
	}
	totalPlayers = amountOfPlayers;
}

void game_manager::resizeView()
{
	sf::Vector2u finViewSize;
	const sf::Vector2u defaultView = sf::Vector2u(viewSize, viewSize);
	// compress the view if it's small than the default window size
	if (window->getSize().x > defaultView.x)
	{
		finViewSize.x = window->getSize().x;
	}
	else
	{
		finViewSize.x = viewSize;
	}
	if (window->getSize().y > defaultView.y)
	{
		finViewSize.y = window->getSize().y;
	}
	else
	{
		finViewSize.y = viewSize;
	}
	// set view port to be the size of the window / compressed window
	view->setSize(static_cast<float>(finViewSize.x), static_cast<float>(finViewSize.y));
	// center the viewport on the game
	view->setCenter(static_cast<float>(viewSize / 2), static_cast<float>(viewSize / 2));
}

void game_manager::handleNetworkMessages()
{
	// get the message queue from the network manager
	std::vector<message> messageQueue = connectionManager->getRecievedMessageQueue();
	for (auto i = 0; i < messageQueue.size(); i++)
	{
		executeMessageCommand(messageQueue[i]);
	}
	// clear the queue once we have handled all messages
	connectionManager->clearRcvQueue();
}

void game_manager::executeMessageCommand(message msg)
{
	switch (msg.getCmdID())
	{
		// case 0 means we need to setup a networked player
		case 0:
		{
			setupPlayers(msg.getClientID(), msg.getMsgVal());
			break;
		}
		// case 1 means we need to update a player's position
		case 1:
		{
			// check if the player is networked
			if(playerList[msg.getClientID()]->getNetworkedPlayer())
			{
				// increment the time by our own latency (to add the time we predict it will take the server message to be recieved
				playerList[msg.getClientID()]->updateIncomingPosition(msg.getMessageVector(), msg.getMsgVal(), elapsedTime, elapsedTime + ((msg.getLatency() + curLatency)), msg.getTimeSent());
			}
			// reset the timer corresponding either to the sockets saved within the server or the client itself
			if (connectionManager->isServer())
			{
				connectionManager->resetTimer(msg.getClientID());
			}
			else
			{
				connectionManager->resetTimer(0);
			}
			break;
		}
		// case 2 means that we need to instantiate a new particle
		case 2: 
		{
			instantiateProjectile(msg.getMsgVec(), msg.getClientID(), static_cast<game_object::LOOK_DIR>(msg.getMsgVal()));
			break;
		}
		// case 3 means that a projectile collided with an object and we want to destroy it
		case 3:
		{
			deactivateProjectile(msg.getMsgVal());
			break;
		}
		// case 4 means a player got hit by a projectile
		case 4:
		{
			deactivateProjectile(msg.getMsgVal());
			handleDeath(msg.getClientID());
			break;
		}
		// case 5 means that we need to initialise a power up
		case 5:
		{
			powerUps[msg.getClientID()]->setupPU(static_cast<powerup::ItemType>(msg.getMsgVal()));
			break;
		}
		// case 6 means a player has collected a power up
		case 6:
		{
			collectPowerUp(msg.getClientID(), msg.getMsgVal());
			break;
		}
		// case 7 means we ahve recieved a ping-poing message
		case 7:
		{
			// reset the timer corresponding either to the sockets saved within the server or the client itself
			if(connectionManager->isServer())
			{
				// reset sender's disconnect timer
				connectionManager->resetTimer(msg.getClientID());
				curLatency = 0.0f;
			}
			else
			{
				// reset local disconnect timer
				connectionManager->resetTimer(0);
				if(msg.getClientID() == localID)
				{
					// we assume that recieving and sending take equal amounts of time, so we need to divide by two get the latency for one trip
					curLatency = (latencyTimer / 2);
					latencyUpdateRequired = true;
					std::cout << curLatency;
				}
			}
			break;
		}
		// case 8 means that another player disconnected from the server
		case 8:
		{
			// if it's the local client, the game is over (our connection was too bad but we got this message for some reason - or perhaps our udp socket broke?)
			if (msg.getClientID() == localID)
			{
				if(connectionManager->isServer() == false)
				{
					gameOver = true;
				}
			}
			else
			{
				// disconnect the player
				playerList[msg.getClientID()]->disconnectPlayer();
				// kill off the player
				handleDeath(msg.getClientID());
			}
			break;
		}
		// case 9 means that the local client has lost connection to the server
		case 9:
		{
			gameOver = true;
			break;
		}
		default:
		{
			break;
		}
	}
}

void game_manager::collectPowerUp(int plID, int powerUpID)
{
	switch (powerUps[powerUpID]->getItemType())
	{
	case powerup::ItemType::speedup:
		// increase the player's movement speed
		playerList[plID]->setFastSpeed();
		break;
	case powerup::ItemType::shotsup:
		// increase the fire rate
		fireRate = fastFireRate;
		break;
	case powerup::ItemType::speeddown:
		// slow each player down that is no the player that collected the item
		for(auto i = 0; i < playerList.size(); i++)
		{
			if(i != plID)
			{
				playerList[i]->setSlowSpeed();
			}
		}
		break;
	}
	// disable the power up
	powerUps[powerUpID]->resetPU();
}


void game_manager::instantiateProjectile(sf::Vector2f instantPos, int playerID, game_object::LOOK_DIR direction)
{
	// create a new projectile with the information passed from the server
	projectile* newProjectile = new projectile(projectileColliderSize, direction, playerID, fireTex, sf::Vector2f(static_cast<float>(levelSize), static_cast<float>(1 + gridSize)), instantPos);
	projectiles.push_back(newProjectile);
}

void game_manager::deactivateProjectile(sf::Int64 projID)
{
	// loop through each projectile
	projectiles[projID]->deactivate();
}

void game_manager::handleDeath(int plID)
{
	int respawnedPlayers = 0;
	playersAlive--;
	for (auto i = 0; i < totalPlayers; i++)
	{
		// kill the player that got hit
		if (i == plID)
		{
			playerList[i]->killPlayer();
		}
		// all players that didn't get hit will score
		else
		{
			// if they are alive
			if (playerList[i]->isAlive())
			{
				// increment their score
				playerList[i]->setScore(playerList[i]->getScore() + scoreIncrement);
			}
		}
		// check if a round is over
		if (playersAlive <= 1)
		{
			if (playerList[i]->isConnected())
			{
				// all players except 1 have been killed - reset players and upgrade round
				playerList[i]->resetPlayer();
				// reset the fire rate
				fireRate = initFireRate;
				if (i == localID && connectionManager->isServer() == false)
				{
					// indicate that we need to send a position update after making the reset
					playerList[i]->setPendingUpdate();
				}
				// add to the number of respanwed players
				respawnedPlayers++;
			}
		}
	}
	// if players have respawned, a new round has started
	if (respawnedPlayers > 0)
	{
		// increment the round counter
		curRound++;
		// check that more than 1 player respawned
		if (respawnedPlayers > 1)
		{
			// check if the game has ended
			if (curRound < nrOfRounds)
			{
				// if not we need to reset our player counter
				playersAlive = respawnedPlayers;
				// clear projectiles
				clearProjectiles();
				// reset power ups
				resetPowerUps();
			}
			else
			{
				// otherwise, indicate that the game has ended
				gameOver = true;
			}
		}
		else
		{
			// otherwise, not enough players have respawned to continue the game
			gameOver = true;
		}
	}
}

std::vector<int> game_manager::getScoreList()
{
	std::vector<int> scoreList;
	// get each player's score
	for(auto i = 0; i < totalPlayers; i++)
	{
		int score = playerList[i]->getScore();
		scoreList.push_back(score);
	}
	// return it
	return scoreList;
}

void game_manager::instantiatePillars(sf::Texture* wallTexture)
{

	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 5)), static_cast<float>((gridSize * 5))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 12)), static_cast<float>((gridSize * 9))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 4)), static_cast<float>((gridSize * 14))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 8)), static_cast<float>((gridSize * 2))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 9)), static_cast<float>((gridSize * 12))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 10)), static_cast<float>((gridSize * 7))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 15)), static_cast<float>((gridSize * 6))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 13)), static_cast<float>((gridSize * 14))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 3)), static_cast<float>((gridSize * 8))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 7)), static_cast<float>((gridSize * 7))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 6)), static_cast<float>((gridSize * 10))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 14)), static_cast<float>((gridSize * 3))));
	pillarPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 2)), static_cast<float>((gridSize * 3))));

	for(auto i = 0; i < pillarPositions.size(); i++)
	{
		// create a wall object
		pillar* pillarBase = new pillar(pillarPositions[i], wallTexture, static_cast<float>((gridSize * 0.8)), pillarColliderSize);
		// add pillars to the vector
		brickPillars.push_back(pillarBase);
	}
}

void game_manager::instantiatePowerUpSpawns(sf::Texture* powerUpSprite)
{
	powerUpSpawnPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 8)), static_cast<float>((gridSize * 5))));
	powerUpSpawnPositions.push_back(sf::Vector2f(static_cast<float>((gridSize *11)), static_cast<float>((gridSize * 11))));
	powerUpSpawnPositions.push_back(sf::Vector2f(static_cast<float>((gridSize * 6)), static_cast<float>((gridSize * 9))));

	for(auto i = 0; i < powerUpSpawnPositions.size(); i++)
	{
		powerup* newPowerUp = new powerup(powerUpSpawnPositions[i], powerUpColliderSize, powerUpSprite);
		powerUps.push_back(newPowerUp);
	}
}

void game_manager::resetPowerUps()
{
	for(auto i = 0; i < powerUps.size(); i++)
	{
		powerUps[i]->resetPU();
		powerUpTimer = 0.0f;
		requirePuRespawn = true;
	}
}
