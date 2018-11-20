#pragma once
#include <SFML\Graphics.hpp>
class animation
{
public:
	animation();
	~animation();

	sf::IntRect update(int collumn, int row_start, float delta_time);
	void init(sf::Texture* texture, sf::Vector2u image_count, float switch_time);
private:
	sf::Vector2u imageCount;
	sf::Vector2u curImage;

	float elapsedAnimTime;
	float switchTime;

	sf::IntRect uvRect;
};

