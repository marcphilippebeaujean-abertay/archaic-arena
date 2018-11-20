#include "animation.h"

animation::animation()
{
	
}

animation::~animation()
{
}

void animation::init(sf::Texture* texture, sf::Vector2u image_count, float switch_time)
{
	// initialise class variables
	this->switchTime = switch_time;
	this->imageCount = image_count;
	// init variables that store information
	elapsedAnimTime = 0.0f;
	curImage.x = 0;
	// setup rect -> size of texture divided by the number of rows and/or collumns on the sprite sheet
	uvRect.width = (texture->getSize().x / static_cast<int>(image_count.x));
	uvRect.height = (texture->getSize().y / static_cast<int>(image_count.y));
}

sf::IntRect animation::update(int collumn, int row_start, float delta_time)
{
	curImage.y = collumn;
	elapsedAnimTime += delta_time;
	// check if it's time to load a new sprite for the animation
	if (elapsedAnimTime >= switchTime)
	{
		// reset elapsed time
		elapsedAnimTime -= switchTime;
		// increment the x value 
		curImage.x++;
		// reset cur image if it has exceeded the number of sprites on the row
		if (curImage.x >= imageCount.x)
		{
			curImage.x = row_start;
		}
	}
	// no need to recalculate the width/height, merely set first two values
	uvRect.left = curImage.x * uvRect.width;
	uvRect.top = curImage.y * uvRect.height;

	return uvRect;
}