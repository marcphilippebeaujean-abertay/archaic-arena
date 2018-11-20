#include "transform.h"
#include "animation.h"
#pragma once
class game_object : public transform
{
public:
	enum class LOOK_DIR
	{
		UP,
		RIGHT,
		DOWN,
		LEFT
	};
	game_object(sf::Vector2f colliderSize);
	~game_object();
	// functions
	virtual void update(float& delta_time);
	void obj_init(sf::Vector2f max_pos, sf::Vector2f min_pos, sf::Vector2f init_pos);
	bool boundryCheck(sf::Vector2f& position);
	void updateObjRotation(LOOK_DIR new_dir);

	// setters
	void setLookDir(LOOK_DIR new_dir) { lookDir = new_dir; }
	
	// getters
	sf::Vector2f getMaxPos() { return maxPos; }
	sf::Vector2f getMinPos() { return minPos; }
	animation* getUpdateAnim() { return updateAnim; }
	LOOK_DIR getLookDir() { return lookDir; }
	int getLookDirNr() { return static_cast<int>(lookDir); }
private:
	sf::Vector2f maxPos;
	sf::Vector2f minPos;
	animation* updateAnim;
	// direction the object is facing
	LOOK_DIR lookDir;
};

