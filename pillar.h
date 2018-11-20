#pragma once
#include "transform.h"
class pillar : public transform
{
public:
	pillar( sf::Vector2f position, sf::Texture* texture, int gridSize, sf::Vector2f colliderSize);
	~pillar();
};

