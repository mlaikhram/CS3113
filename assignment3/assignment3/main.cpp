#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "assignment1.app/Contents/Resources/"
#endif

#define PI 3.141592653589793238

SDL_Window* displayWindow;


class Vector3 {
public:
	Vector3() { Vector3(0, 0, 0); }
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;

};

class SheetSprite {
public:
	SheetSprite() { SheetSprite(0.0f, 0); }
	SheetSprite(float size, unsigned int textureID, float u = 0, float v = 0, float width = 0, float height = 0) : size(size), textureID(textureID), u(u), v(v), width(width), height(height) {}

	void Draw(ShaderProgram *program) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size };
		// draw our arrays
		glBindTexture(GL_TEXTURE_2D, textureID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

class Entity {
public:
	Entity() { Entity(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, SheetSprite()); }
	Entity(Vector3 position, Vector3 velocity, Vector3 size, float rotation, SheetSprite sprite) : position(position), velocity(velocity), size(size), rotation(rotation), sprite(sprite) {}
	//void Draw();
	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	float rotation;
	SheetSprite sprite;
};


GLuint LoadTexture(const char *image_path) 
{
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
	int spriteCountY, GLuint textureID) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;

	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float vertices[] = { -0.2f, -0.2f, 0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 0.2f, -0.2f, -0.2f, 0.2f, -0.2f };
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Espurr Invasion", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	
	float lastFrameTicks = 0.0f;
	
	GLuint spriteSheet = LoadTexture("spritesheet.png");
	GLuint letters = LoadTexture("letters.png");
	enum letterIndex { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z };
	int LETTER_SHIFT = 65;
	int message[] = { P, R, E, S, S, -1, S, P, A, C, E, -1, T, O, -1, B, E, G, I, N };

	SheetSprite espurrSprite = SheetSprite(0.4f, spriteSheet, 5.0f / 2446.0f, 5.0f / 2010.0f, 250.0f / 2446.0f, 250.0f / 2010.0f);
	SheetSprite bulletSprite = SheetSprite(0.1f, spriteSheet, 265.0f / 2446.0f, 5.0f / 2010.0f, 2000.0f / 2446.0f, 2000.0f / 2010.0f);
	SheetSprite squirtleSprite = SheetSprite(0.4f, spriteSheet, 2275.0f / 2446.0f, 5.0f + 2000.0f / 2010.0f, 166.0f / 2446.0f, 200.0f / 2010.0f);

	Entity player(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, squirtleSprite);

	std::vector<Entity> espurrs;
	for (int i = 0; i < 17 * 3; ++i) {
		Entity espurr(Vector3(-3.2f + (i % 17) * 0.4f, 2.0f - 0.2f - 0.4f * (i / 17), 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, espurrSprite);
		espurrs.push_back(espurr);
	}

	std::vector<Entity> bullets;
	for (int i = 0; i < 25; ++i) {
		Entity bullet(Vector3(0.55, 2.1f, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, bulletSprite);
		bullets.push_back(bullet);
	}

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);


	float p1x = 0;
	bool p1shoot = false;
	int bulletCooldown = 0;
	float espurrx = 0;
	float espurrSpeed = 0.05;
	int inactiveBullet = 6; //current inactive bullet index
	enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
	int state = STATE_MAIN_MENU;
	int keyState = 0;

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		switch (state) {

		case STATE_MAIN_MENU:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						state = STATE_GAME_LEVEL;
					}
				}
			}
			glClear(GL_COLOR_BUFFER_BIT);
			
			for (int i = 0; i < 20; ++i) {
				if (message[i] != -1) {
					modelMatrix.identity();
					modelMatrix.Translate(-3.45 + i*.2 + 1.4, 0, 0);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(&program, message[i] + 65, 16, 16, letters);
				}
			}
			SDL_GL_SwapWindow(displayWindow);
			break;

		case STATE_GAME_LEVEL:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						keyState = 1.5;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						keyState = -1.5;
					}
				}
				else {
					keyState = 0;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN && bulletCooldown <= 0) {
					if (event.button.button == 1) {
						p1shoot = true;
					}
				}
				else if (event.type == SDL_MOUSEBUTTONUP) {
					if (event.button.button == 1) {
						p1shoot = false;
					}
				}
			}

			//check bounds for p1
			if (0.2 + p1x + elapsed * keyState > 3.55) {
				p1x = 3.35;
			}
			else if (-0.2 + p1x + elapsed * keyState < -3.55) {
				p1x = -3.35;
			}
			else {
				p1x += elapsed * keyState;
			}

			//check bounds for espurrs
			if (espurrx + elapsed * espurrSpeed > 0.15) {
				espurrSpeed *= -1;
			}
			else if (espurrx + elapsed * espurrSpeed < -0.15) {
				espurrSpeed *= -1;
			}
			espurrx += elapsed * espurrSpeed;

			//check for bullets
			//save first 5 for player
			inactiveBullet = -1;
			for (int i = 5; i < bullets.size(); ++i) {
				if (bullets[i].position.y >= 2.1f) {
					inactiveBullet = i;
					break;
				}
				else {
					inactiveBullet = -1;
				}
			}

			//espurr shoot
			if (inactiveBullet != -1 && (int)(ticks * 1000) % 5 == 0) {
				int shooter = std::rand() % espurrs.size();
				bullets[inactiveBullet].position.x = espurrs[shooter].position.x;
				bullets[inactiveBullet].position.y = espurrs[shooter].position.y;
			}

			//check if player shoots
			if (p1shoot) {
				inactiveBullet = -1;
				for (int i = 0; i < 5; ++i) {
					if (bullets[i].position.y >= 2.1f) {
						inactiveBullet = i;
						break;
					}
					else {
						inactiveBullet = -1;
					}
				}
				if (inactiveBullet != -1) {
					bullets[inactiveBullet].position.x = p1x;
					bullets[inactiveBullet].position.y = -1.8;
					bulletCooldown = 10;
					p1shoot = false;
				}
			}
			else {
				--bulletCooldown;
			}
			//reset bullets
			for (int i = 0; i < bullets.size(); ++i) {
				if (bullets[i].position.y > 2.0f || bullets[i].position.y < -2.0f) {
					//bullets[i].position.x = 3.56;
					bullets[i].position.y = 2.1f;
					bullets[i].velocity.y = 0.0f;

				}
			}

			glClear(GL_COLOR_BUFFER_BIT);

			//player
			modelMatrix.identity();
			modelMatrix.Translate(p1x, -1.8, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			player.sprite.Draw(&program);

			//Espurrs
			for (int i = 0; i < espurrs.size(); ++i) {
				modelMatrix.identity();
				modelMatrix.Translate(espurrs[i].position.x + espurrx, espurrs[i].position.y, 0);
				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);

				espurrs[i].sprite.Draw(&program);
			}

			//bullets
			for (int i = 0; i < bullets.size(); ++i) {
				if (i < 5 && bullets[i].position.y < 2.1f) {
					bullets[i].velocity.y = 1;
				}
				else if (i >= 5 && bullets[i].position.y < 2.1f) {
					bullets[i].velocity.y = -1;
				}
				bullets[i].position.y += elapsed * bullets[i].velocity.y;

				modelMatrix.identity();
				modelMatrix.Translate(bullets[i].position.x, bullets[i].position.y, 0);
				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);

				bullets[i].sprite.Draw(&program);
			}

			//collisions
			bool breakout = false;
			for (int i = 0; i < espurrs.size(); ++i) {
				for (int j = 0; j < 5; ++j) {
					if (!(bullets[j].position.y + 0.05 < espurrs[i].position.y - 0.2f ||
						bullets[j].position.y - 0.05 > espurrs[i].position.y + 0.2f ||
						bullets[j].position.x + 0.05 < espurrs[i].position.x - 0.2f ||
						bullets[j].position.x - 0.05 > espurrs[i].position.x + 0.2f
						)) {
						espurrs.erase(espurrs.begin() + i);
						bullets[j].position.y = 2.1f;
						bullets[j].velocity.y = 0.0f;
						bullets[j].velocity.x = 1.0f;
						breakout = true;
						break;
					}
				}
				if (breakout) break;
			}
			for (int i = 5; i < bullets.size(); ++i) {
				if (espurrs.size() == 0 || 
					!(bullets[i].position.y + 0.05 < -1.8f - 0.2f ||
					bullets[i].position.y - 0.05 > -1.8f + 0.2f ||
					bullets[i].position.x + 0.05 < p1x - 0.1f ||
					bullets[i].position.x - 0.05 > p1x + 0.1f
					)) {
					//reset
					player = Entity(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, squirtleSprite);
					
					espurrs.clear();
					for (int i = 0; i < 17 * 3; ++i) {
						Entity espurr(Vector3(-3.2f + (i % 17) * 0.4f, 2.0f - 0.2f - 0.4f * (i / 17), 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, espurrSprite);
						espurrs.push_back(espurr);
					}

					bullets.clear();
					for (int i = 0; i < 25; ++i) {
						Entity bullet(Vector3(0.55, 2.1f, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, bulletSprite);
						bullets.push_back(bullet);
					}
					p1x = 0;
					p1shoot = false;
					bulletCooldown = 0;
					espurrx = 0;
					espurrSpeed = 0.05;
					inactiveBullet = 6;
					state = STATE_MAIN_MENU;
					keyState = 0;
					break;
				}
			}


			SDL_GL_SwapWindow(displayWindow);
			break;
		}
	}

	SDL_Quit();
	return 0;
}