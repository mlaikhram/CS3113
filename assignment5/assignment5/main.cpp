#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

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
	
	void normalize2d() {
		float mag = sqrt(x*x + y*y);
		if (mag != 0) {
			x /= mag;
			y /= mag;
		}
	}

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
	//Entity() { Entity(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, SheetSprite()); }
	//Entity(Vector3 position, Vector3 velocity, Vector3 size, float rotation, SheetSprite sprite) : position(position), velocity(velocity), size(size), rotation(rotation), sprite(sprite) {}
	Entity(std::string type, bool isStatic, Vector3 position, Vector3 velocity = Vector3(0, 0, 0), Vector3 acceleration = Vector3(0, 0, 0), Vector3 size = Vector3(0, 0, 0), float rotation = 0.0f) : 
		type(type), isStatic(isStatic), position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), collidedTop(false), collidedBottom(false), collidedLeft(false), collidedRight(false) {}
	//void Draw();
	std::string type;
	bool isStatic;
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 size;
	float rotation;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
	std::vector<Vector3> edges;
	//SheetSprite sprite;
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

float TILE_SIZE = .2;

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
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

	float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE };

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

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

bool entityCollision(Entity e1, Entity e2) {
	int top = 0;
	if (e1.type == "player") {
		top = TILE_SIZE;
	}
	return (!(
		e1.position.x - TILE_SIZE / 2 > e2.position.x + TILE_SIZE / 2 ||
		e1.position.x + TILE_SIZE / 2 < e2.position.x - TILE_SIZE / 2 ||
		e1.position.y - TILE_SIZE / 2 > e2.position.y + TILE_SIZE / 2 ||
		e1.position.y + TILE_SIZE / 2 + top < e2.position.y - TILE_SIZE / 2));
}

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector3> &points1, const std::vector<Vector3> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector3> &e1Points, const std::vector<Vector3> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	//displayWindow = SDL_CreateWindow("MAGNETIC MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	displayWindow = SDL_CreateWindow("SHOOTING STAR", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//glViewport(0, 0, 640, 360);
	glViewport(0, 0, 1280, 720);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	
	float lastFrameTicks = 0.0f;
	
	//GLuint spriteSheet = LoadTexture("spritesheet.png");
	GLuint letters = LoadTexture("letters.png");
	GLuint goldenGearSpriteSheet = LoadTexture("golden_gear_spritesheet.png");
	enum letterIndex { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z };
	int LETTER_SHIFT = 65;
	int message[] = { P, R, E, S, S, -1, S, P, A, C, E, -1, T, O, -1, B, E, G, I, N };
	std::vector<Vector3> coorList = { Vector3(-0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0), Vector3(-0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0), Vector3(0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0), Vector3(0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0) };

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
	int state = STATE_MAIN_MENU;

	float a = 0;
	float rot = 0;
	std::vector<Entity> entities;
	Entity player("star", false, Vector3(0, 0, 0));
	Entity crab1("crab", false, Vector3(-1/ 3.55, 0, 0));
	Entity crab2("crab", false, Vector3(1 / 3.55, 0, 0));
	crab2.rotation = PI / 4;
	entities.push_back(player);
	entities.push_back(crab1);
	entities.push_back(crab2);
	for (int i = 0; i < entities.size(); ++i) {
		entities[i].edges = coorList;
		for (int j = 0; j < coorList.size(); ++j) {
			entities[i].edges[j].x += entities[i].position.x;
			entities[i].edges[j].y += entities[i].position.y;
		}
	}

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		// 60 FPS (1.0f/60.0f)
		#define FIXED_TIMESTEP 0.0166666f
		#define MAX_TIMESTEPS 6
		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			//Update(FIXED_TIMESTEP);
		}
		//Update(fixedElapsed);

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
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_UP || event.key.keysym.scancode == SDL_SCANCODE_O || event.key.keysym.scancode == SDL_SCANCODE_W) {
						//player.acceleration.y = 5;
						a = 5;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_SEMICOLON || event.key.keysym.scancode == SDL_SCANCODE_D) {
						//player.rotation -= 1;
						rot -= 0.1;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_K || event.key.keysym.scancode == SDL_SCANCODE_A) {
						//player.rotation += 1;
						rot += 0.1;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						//p1vy = 3.1;
					}
				}
				else if (event.type == SDL_KEYUP) {
					if (event.key.keysym.scancode == SDL_SCANCODE_UP || event.key.keysym.scancode == SDL_SCANCODE_O || event.key.keysym.scancode == SDL_SCANCODE_W) {
						player.acceleration.y = 0;
						a = 0;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						//p1vy = 0;
					}
				}
			}

			////////MOVEMENT//////////////////////////////////////////////////////////////////////////////////
			for (int i = 0; i < entities.size(); ++i) {
				
				entities[i].collidedTop = false;
				entities[i].collidedBottom = false;
				entities[i].collidedLeft = false;
				entities[i].collidedRight = false;

				if (i == 0) {
					entities[i].rotation = rot;
					entities[i].acceleration.x = -a * sin(rot);
					entities[i].acceleration.y = a * cos(rot);
				}

				entities[i].velocity.x = lerp(entities[i].velocity.x, 0.0f, FIXED_TIMESTEP * 1);
				entities[i].velocity.y = lerp(entities[i].velocity.y, 0.0f, FIXED_TIMESTEP * 1);

				entities[i].velocity.x += entities[i].acceleration.x * FIXED_TIMESTEP;
				entities[i].velocity.y += entities[i].acceleration.y * FIXED_TIMESTEP;

				entities[i].position.x += entities[i].velocity.x * FIXED_TIMESTEP;
				entities[i].position.y += entities[i].velocity.y * FIXED_TIMESTEP;

			}

			//update edge lists
			for (int i = 0; i < entities.size(); ++i) {
				entities[i].edges = coorList;
				for (int j = 0; j < coorList.size(); ++j) {
					entities[i].edges[j].x += entities[i].position.x;
					entities[i].edges[j].y += entities[i].position.y;
				}
			}

			////////ENTITY COLLISION//////////////////////////////////////////////////////////////////////////
			for (int i = 0; i < entities.size(); ++i) {
				for (int j = i + 1; j < entities.size(); ++j) {
					int maxChecks = 10;
					while (checkSATCollision(entities[i].edges, entities[j].edges) && maxChecks > 0) {
						Vector3 responseVector = Vector3(entities[i].position.x - entities[j].position.x, entities[i].position.y - entities[j].position.y, 0);
						responseVector.normalize2d();
						entities[i].position.x += responseVector.x * 0.002;
						entities[i].position.y += responseVector.y * 0.002;

						entities[j].position.x -= responseVector.x * 0.002;
						entities[j].position.y -= responseVector.y * 0.002;
						maxChecks -= 1;
					}
				}
			}

			///////DRAWING////////////////////////////////////////////////////////////////////////////////////
			glClear(GL_COLOR_BUFFER_BIT);

			std::vector<std::string> types = {"gear", "goldenGear", "crab", "star", "bullet"};
			for (int i = 0; i < entities.size(); ++i) {
				modelMatrix.identity();
				modelMatrix.Translate(entities[i].position.x, entities[i].position.y, 0);
				modelMatrix.Rotate(entities[i].rotation);
				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);

				int pos = find(types.begin(), types.end(), entities[i].type) - types.begin();
				DrawSpriteSheetSprite(&program, pos + 24, 20, 10, goldenGearSpriteSheet);
				viewMatrix.identity();
				viewMatrix.Translate(-entities[0].position.x, -1 * (entities[0].position.y), 0);
			}
			SDL_GL_SwapWindow(displayWindow);
			break;
		}
	}

	SDL_Quit();
	return 0;
}