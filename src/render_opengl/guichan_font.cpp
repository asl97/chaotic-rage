// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include "guichan_font.h"
#include "gl.h"
#include "gl_debug.h"
#include "render_opengl.h"
#include "../rage.h"
#include "../mod/mod.h"
#include "../util/utf8.h"
#include "../util/util.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


/**
* Metadata and texture pointer for freetype characters
**/
class FreetypeChar
{
	public:
		GLuint tex;
		int x, y, w, h;
		float tx, ty;
		int advance;

	public:
		FreetypeChar()
			: tex(0), x(0), y(0), w(0), h(0), tx(0.0f), ty(0.0f), advance(0)
			{}

		~FreetypeChar()
		{
			if (tex) glDeleteTextures(1, &tex);
		}
};


/**
* Implementation PMPL to save lots of headers
**/
class OpenGLFont_Implementation
{
	public:
		GLuint font_vbo;
		FT_Library ft;
		FT_Face face;
		Uint8 *buf;
		map<Uint32, FreetypeChar> char_tex;
};


/**
* Init and load font
**/
OpenGLFont::OpenGLFont(RenderOpenGL* render, string name, Mod* mod, float size)
{
	int error;
	Sint64 len;

	// Basics
	this->pmpl = new OpenGLFont_Implementation();
	this->render = render;
	this->size = size;

	// Init freetype
	// TODO: Should we share the tf ptr between OpenGLFont instances
	error = FT_Init_FreeType(&this->pmpl->ft);
	if (error) {
		reportFatalError("Freetype: Unable to init library.");
	}

	// Load file from mod
	this->pmpl->buf = mod->loadBinary(name, &len);
	if (this->pmpl->buf == NULL) {
		reportFatalError("Freetype: Unable to load data");
	}

	// Load face
	error = FT_New_Memory_Face(this->pmpl->ft, (const FT_Byte *) this->pmpl->buf, (FT_Long)len, 0, &this->pmpl->face);
	if (error == FT_Err_Unknown_File_Format) {
		reportFatalError("Freetype: Unsupported font format");
	} else if (error) {
		reportFatalError("Freetype: Unable to load font");
	}

	// Set character size
	error = FT_Set_Char_Size(this->pmpl->face, 0, size * 64, 72, 72);
	if (error) {
		reportFatalError("Freetype: Unable to load font size");
	}
}


/**
* Cleanup
**/
OpenGLFont::~OpenGLFont()
{
	this->pmpl->char_tex.clear();
	FT_Done_Face(this->pmpl->face);
	FT_Done_FreeType(this->pmpl->ft);
	free(this->pmpl->buf);
	glDeleteBuffers(1, &this->pmpl->font_vbo);
	delete this->pmpl;
}


/**
* Gets the width of a string. The width of a string is not necesserily
* the sum of all the widths of it's glyphs.
*
* @param text The string to return the width of.
* @return The width of a string.
*/
int OpenGLFont::getWidth(const std::string& text) const
{
	unsigned int w = 0;

	const char* ptr = text.c_str();
	size_t textlen = strlen(ptr);
	while (textlen > 0) {
		Uint32 c = UTF8_getch(&ptr, &textlen);
		if (c == UNICODE_BOM_NATIVE || c == UNICODE_BOM_SWAPPED) {
			continue;
		}

		FT_GlyphSlot slot = this->pmpl->face->glyph;

		// Load glyph image into the slot
		int error = FT_Load_Char(this->pmpl->face, c, FT_LOAD_DEFAULT);
		if (error) continue;

		w += (slot->advance.x >> 6);
	}

	return w;
}


/**
* Gets the height of the glyphs in the font.
*
* @return The height of the glyphs int the font.
*/
int OpenGLFont::getHeight() const
{
	return (int)round(this->size * 1.3f);
}


/**
* Draws a string.
*
* NOTE: You normally won't use this function to draw text since
*       Graphics contains better functions for drawing text.
*
* @param graphics A Graphics object to use for drawing.
* @param text The string to draw.
* @param x The x coordinate where to draw the string.
* @param y The y coordinate where to draw the string.
*/
void OpenGLFont::drawString(gcn::Graphics* graphics, const std::string& text, int x, int y)
{
	this->drawString(graphics, text, x, y, 0.0f, 0.0f, 0.0f, 1.0f);
}


/**
* Draws a string.
*
* NOTE: You normally won't use this function to draw text since
*       Graphics contains better functions for drawing text.
*
* @param graphics A Graphics object to use for drawing.
* @param text The string to draw.
* @param x The x coordinate where to draw the string.
* @param y The y coordinate where to draw the string.
*/
void OpenGLFont::drawString(gcn::Graphics* graphics, const std::string& text, int x, int y, float r, float g, float b, float a)
{
	float xx, yy;

	if (this->pmpl->font_vbo == 0) {
		glGenBuffers(1, &this->pmpl->font_vbo);
	}

	#ifdef OpenGL
		glBindVertexArray(0);
	#endif

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	// Set up shader
	GLShader* shader = this->render->shaders["text"];
	glUseProgram(shader->p());
	glUniform1i(shader->uniform("uTex"), 0);
	glUniform4f(shader->uniform("uColor"), r, g, b, a);
	glUniformMatrix4fv(shader->uniform("uMVP"), 1, GL_FALSE, glm::value_ptr(this->render->ortho));

	// Determine starting X and Y coord
	if (graphics == NULL) {
		xx = x;
		yy = y;
	} else {
		const gcn::ClipRectangle& top = graphics->getCurrentClipArea();
		xx = x + top.xOffset;
		yy = y + top.yOffset + this->size;
	}

	// Render each character
	const char* ptr = text.c_str();
	size_t textlen = strlen(ptr);
	while (textlen > 0) {
		Uint32 c = UTF8_getch(&ptr, &textlen);
		if (c == UNICODE_BOM_NATIVE || c == UNICODE_BOM_SWAPPED) {
			continue;
		}
		this->renderCharacter(c, xx, yy);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	CHECK_OPENGL_ERROR;
}


/**
* Draws a single character of text
* Called by ::renderText - you probably want that function instead
*
* @param Uint32 character A 32-bit character code
**/
void OpenGLFont::renderCharacter(Uint32 character, float &x, float &y)
{
	FreetypeChar *c = &(this->pmpl->char_tex[character]);

	// If the OpenGL tex does not exist for this character, create it
	if (c->tex == 0) {
		FT_GlyphSlot slot = this->pmpl->face->glyph;

		int error = FT_Load_Char(this->pmpl->face, character, FT_LOAD_DEFAULT);
		if (error) return;

		error = FT_Render_Glyph(this->pmpl->face->glyph, FT_RENDER_MODE_NORMAL);
		if (error) return;

		int width = nextPowerOfTwo(slot->bitmap.width);
		int height = nextPowerOfTwo(slot->bitmap.rows);

		GLubyte* gl_data = new GLubyte[2 * width * height];

		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {

				gl_data[2*(i+j*width)] = gl_data[2*(i+j*width)+1] =
					(i>=slot->bitmap.width || j>=slot->bitmap.rows) ?
					0 : slot->bitmap.buffer[i + slot->bitmap.width*j];

			}
		}

		// Create a texture
		glGenTextures(1, &c->tex);
		glBindTexture(GL_TEXTURE_2D, c->tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		#ifdef OpenGL
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, gl_data);

		delete [] gl_data;

		c->w = slot->bitmap.width;
		c->h = slot->bitmap.rows;
		c->x = slot->bitmap_left;
		c->y = slot->bitmap_top;
		c->advance = slot->advance.x;
		c->tx = (float)slot->bitmap.width / (float)width;
		c->ty = (float)slot->bitmap.rows / (float)height;

	} else {
		glBindTexture(GL_TEXTURE_2D, c->tex);
	}

	GLfloat box[4][4] = {
		{x + c->x,         y + -c->y + c->h,  0.f,    c->ty},
		{x + c->x + c->w,  y + -c->y + c->h,  c->tx,  c->ty},
		{x + c->x,         y + -c->y,         0.f,    0.f},
		{x + c->x + c->w,  y + -c->y,         c->tx,  0.f},
	};

	glBindBuffer(GL_ARRAY_BUFFER, this->pmpl->font_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(ATTRIB_TEXTCOORD, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(ATTRIB_TEXTCOORD);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	x += (float)(c->advance >> 6);
}

