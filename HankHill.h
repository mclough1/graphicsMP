#include "Hero.h"

class HankHill : public Hero {
	public:
		void draw();
		void recompute();
		
		glm::vec3 pos;
		float theta;
	
	private:
		float teapotScale = 1.0f, teapotScaleFactor = .01f;
	
};