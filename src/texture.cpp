#include "common.h"
#include <vector>

std::vector<texture_t*> textures;
unsigned int actualCount = 1;

void Texture_Unload_All()
{
	textures.clear();
	actualCount = 1;
}

void Texture_Draw(GLuint tex, float w, float h, float x, float y, int debug)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	glOrtho(0, 800, 600, 0, 0, 100);
	glTranslatef(x, y, 0);

	glBegin(GL_QUADS);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w, 0, 0.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0, 0, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0, h, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w, h, 0.0f);

	glEnd();

	glDisable(GL_TEXTURE_2D);

	if (debug > 0)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		glVertex3f(w, 0, 0);
		glVertex3f(0, 0, 0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(0, h, 0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(0, h, 0);
		glVertex3f(w, h, 0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(w, h, 0);
		glVertex3f(w, 0, 0);
		glEnd();
	}
}

texture_t* Texture_Get(unsigned int id)
{
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		if (textures.at(i)->textureId == id)
		{
			return textures.at(i);
		}
	}
	return 0;
}

GLuint Texture_Load(char* nameFile)
{
	GLuint texture;
	int mode;
	SDL_Surface* surface;
	texture_t *tex;

	surface = IMG_Load(nameFile);
	if (surface == NULL)
	{
		Print_Error(1, "Cannot load image %s", nameFile);
		return NULL;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	mode = GL_RGB;

	if (surface->format->BytesPerPixel == 4) {
		mode = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	tex = new texture_t;
	tex->textureId = texture;
	tex->w = surface->w;
	tex->h = surface->h;
	tex->name = CopyString(nameFile);

	textures.push_back(tex);

	SDL_FreeSurface(surface);

	Print("Loading %s", nameFile);

	return texture;
}