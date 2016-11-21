#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "assignment1.app/Contents/Resources/"
#endif

#define PI 3.141592653589793238

SDL_Window* displayWindow;

class Entity {
public:
	void Draw();
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;
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


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);		//sound initialization
	Mix_Chunk *airhorn;
	airhorn = Mix_LoadWAV("Airhorn.wav");
	Mix_Chunk *sniped;
	sniped = Mix_LoadWAV("Sniped.wav");
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	float lastFrameTicks = 0.0f;

	GLuint p1Texture = LoadTexture("gray.png");
	GLuint p2Texture = LoadTexture("gray.png");
	GLuint ballTexture = LoadTexture("espurr.png");
	GLuint crownTexture = LoadTexture("crown.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float p1y = 0;
	float p2y = 0;
	float ballx = 0;
	float bally = 0;
	int angle = std::rand() % 178 + 91;
	int winnerx = 0;
	int mouseState = 0;
	int keyState = 0;

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
					keyState = 1.5;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					keyState = -1.5;
				}
			}
			else {
				keyState = 0;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == 1) {
					mouseState = 1.5;
				}
				else if (event.button.button == 3) {
					mouseState = -1.5;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				mouseState = 0;
			}
		}

		//check bounds for p1
		if (0.4 + p1y + elapsed * keyState > 2.0) {
			p1y = 1.6;
		}
		else if (-0.4 + p1y + elapsed * keyState < -2.0) {
			p1y = -1.6;
		}
		else {
			p1y += elapsed * keyState;
		}

		//check bounds for p2
		if (0.4 + p2y + elapsed * mouseState > 2.0) {
			p2y = 1.6;
		}
		else if (-0.4 + p2y + elapsed * mouseState < -2.0) {
			p2y = -1.6;
		}
		else {
			p2y += elapsed * mouseState;
		}

		//update ball position
		if ((0.05 + bally) > 2.0 || (-0.05 + bally) < -2.0) {
			angle *= -1;
		}
		else if (!((-0.4 + p1y) > (0.05 + bally) || (0.4 + p1y) < (-0.05 + bally) || -3.45 < (-0.05 + ballx)) ||
			!((-0.4 + p2y) > (0.05 + bally) || (0.4 + p2y) < (-0.05 + bally) || 3.45 > (0.05 + ballx))) {
			Mix_PlayChannel(-1, sniped, 0);		//SNIPED
			angle = (180 - angle) % 360;
		}

		ballx += elapsed * 1.5 * cos(float(angle) * PI / 180.0f);
		bally += elapsed * 1.5 * sin(float(angle) * PI / 180.0f);

		//check for victory
		if (-3.55 > ballx) {
			winnerx = 2;
			ballx = 0;
			bally = 0;
			Mix_PlayChannel(-1, airhorn, 0);		//airhorn
		}
		else if (3.55 < ballx) {
			winnerx = -2;
			ballx = 0;
			bally = 0;
			Mix_PlayChannel(-1, airhorn, 0);		//airhorn
		}

		glClear(GL_COLOR_BUFFER_BIT);

		//p1
		modelMatrix.identity();
		modelMatrix.Translate(-3.5, p1y, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, p1Texture);
		//float vertices[] = { -3.55, -0.4 + p1y, -3.45, -0.4 + p1y, -3.45, 0.4 + p1y, -3.55, -0.4 + p1y, -3.45, 0.4 + p1y, -3.55, 0.4 + p1y };
		float vertices[] = { -0.05, -0.4, 0.05, -0.4, 0.05, 0.4, -0.05, -0.4, 0.05, 0.4, -0.05, 0.4 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//p2
		modelMatrix.identity();
		modelMatrix.Translate(3.5, p2y, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, p2Texture);
		//float vertices2[] = { 3.45, -0.4 + p2y, 3.55, -0.4 + p2y, 3.55, 0.4 + p2y, 3.45, -0.4 + p2y, 3.55, 0.4 + p2y, 3.45, 0.4 + p2y };
		//float vertices2[] = { -0.05, -0.4, 0.05, -0.4, 0.05, 0.4, -0.05, -0.4, 0.05, 0.4, -0.05, 0.4 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//crown
		modelMatrix.identity();
		modelMatrix.Translate(winnerx, 0, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, crownTexture);
		//float vertices4[] = { -0.3 + winnerx, -0.3, 0.3 + winnerx, -0.3, 0.3 + winnerx, 0.3, -0.3 + winnerx, -0.3, 0.3 + winnerx, 0.3, -0.3 + winnerx, 0.3 };
		float vertices4[] = { -0.3, -0.3, 0.3, -0.3, 0.3, 0.3, -0.3, -0.3, 0.3, 0.3, -0.3, 0.3 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices4);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords4[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords4);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//ball
		modelMatrix.identity();
		modelMatrix.Translate(ballx, bally, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, ballTexture);
		//float vertices3[] = { -0.05 + ballx, -0.05 + bally, 0.05 + ballx, -0.05 + bally, 0.05 + ballx, 0.05 + bally, -0.05 + ballx, -0.05 + bally, 0.05 + ballx, 0.05 + bally, -0.05 + ballx, 0.05 + bally };
		float vertices3[] = { -0.05, -0.05, 0.05, -0.05, 0.05, 0.05, -0.05, -0.05, 0.05, 0.05, -0.05, 0.05 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		SDL_GL_SwapWindow(displayWindow);
	}

	Mix_FreeChunk(airhorn);
	SDL_Quit();
	return 0;
}