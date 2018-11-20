#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "networked_player.h"
#include "file_manager.h"
#include "connection_manager.h"
#include "message.h"
#include "pillar.h"
#include "projectile.h"
#include "powerup.h"
#include "Thor/Math.hpp"

class game_manager
{
public:

	game_manager(file_manager* fileManager);
	~game_manager();

	// run-time management
	void resizeView();
	void update(float& deltaTime);
	void render();

	// getters
	sf::RenderWindow* getWindow() { return window; }
	sf::View* getView() { return view; }
	bool& isGameOver() { return gameOver; }
	std::vector<int> getScoreList();
	int& getID() { return localID; }

	// setters
	void setConnectionManager(connection_manager* connectionManager) { this->connectionManager = connectionManager; }

private:

	// level dependancies
	const int levelSize = 512;
	const int aspectMod = 30;
	const int gridSize = 32;
	int viewSize;

	// render dependancies
	sf::RenderWindow* window;
	sf::View* view;

	// world objects
	std::vector<player*> playerList;
	std::vector<sf::RectangleShape*> environmentAssets;
	std::vector<pillar*> brickPillars;
	std::vector<projectile*> projectiles;
	std::vector<powerup*> powerUps;

	// spawn positions of world objects
	std::vector<sf::Vector2f> pillarPositions;
	std::vector<sf::Vector2f> powerUpSpawnPositions;

	void instantiatePowerUpSpawns(sf::Texture* powerUpSprite);
	void instantiatePillars(sf::Texture* wallTexture);

	// texture references from the file manager
	sf::Texture* playerTexture;
	sf::Texture* enemyTexture;
	sf::Texture* fireTex;

	// pointer to a circle object in the stored in the filemanager, which can then be drawn for collider debug purposes
	sf::Shape* debugShape;

	// total time that our game has been running for, used to compare the reception time of protocol messages
	float elapsedTime;
	// a variable storing when the game manager last sent out a message call
	float lastCallTime;
	// a variable that indicates how often the game manager should queue message calls
	const float networkUpdateRate = 0.1f;

	// a pointer to the connection manager to handle networked events
	connection_manager* connectionManager;

	// function to handle network messages locally
	void handleNetworkMessages();

	// function that iterates through each messages possible meaning and implements it
	void executeMessageCommand(message msg);

	// stores the ID of the player we are currently controlling
	int localID;

	// networking calls
	void setupPlayers(const int playerID, const sf::Int64 amnountOfPlayers);
	void instantiateProjectile(sf::Vector2f initPos, int playerID, game_object::LOOK_DIR direction);
	void deactivateProjectile(sf::Int64 projID);
	void handleDeath(int plID);

	// amount that gets added onto the score when a player dies
	const int scoreIncrement = 10;

	// variables used to keep track of the player states
	int playersAlive = 0;
	int totalPlayers = 0;

	// variables used to keep tra´ck of the game states
	int curRound = 0;
	const int nrOfRounds = 3;
	bool gameOver = false;

	// generate collider sizes
	const sf::Vector2f plColliderSize = sf::Vector2f(15.0f, 15.0f);
	const sf::Vector2f pillarColliderSize = sf::Vector2f(12.0f, 12.0f);
	const sf::Vector2f projectileColliderSize = sf::Vector2f(5.0f, 6.0f);
	const sf::Vector2f powerUpColliderSize = sf::Vector2f(5.0f, 13.0f);

	// projectile dependancies
	void clearProjectiles();
	const float initFireRate = 1.0f;
	const float fastFireRate = 0.6f;
	float fireRate;
	float fireTimer;

	// power up dependancies
	const float powerUpRespawnTime = 5.0f;
	float powerUpTimer;
	void resetPowerUps();
	void collectPowerUp(int plID, int powerUpID);
	bool requirePuRespawn;

	// rate at which we send "ping.pong" messages
	const float pingPongRate = 0.5f;
	float pingPongTimer = 0.0f;
	
	bool latencyUpdateRequired = false;

	// latency detection dependancies
	float latencyTimer;
	float curLatency;
};

