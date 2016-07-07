// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once

class Heightmap;
class RenderOpenGL;
class GLShader;


class RendererHeightmap {
	private:
		Heightmap* heightmap;
		GLShader* shader;

	public:
		RendererHeightmap(RenderOpenGL* render, Heightmap* heightmap);
		~RendererHeightmap();
		void draw(RenderOpenGL* render);
};
