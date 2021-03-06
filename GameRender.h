#pragma once

#include <Vector>
#include <cstddef>
#include "Renderer.h"

struct FontData;

struct Vert
{
	float x, y;
	float u, v;
};

// contains game specific rendering data.
class GameRender
{
public:

	GameRender(Renderer& r);

	Renderer& renderer;

	GpuImage ralewayFont;
	GpuImage inconsolataFont;

	GpuBuffer uniformBuffer;
	GpuBuffer vertexBuffer;
	int vertIndex = 0;
	Pipeline regularPipeline;
	std::vector<vk::UniqueDescriptorSet> descriptors;

	static constexpr int maxGlyphs = 1000;
	static constexpr int maxVertices = maxGlyphs * 6;

	void CreateVertexBuffer();

	void CreateUniformBuffer();

	void UpdateUniformBuffer();

	void ResetVertexBuffer();

	void DrawVerts(vk::CommandBuffer cmdBuffer, Vert* verts, int vertCount);

	void DrawText(vk::CommandBuffer cmdBuffer, char* str, sf::Vector2f position, FontData& font);
	void Init(std::byte* ralewayBitmap, std::byte* inconsolataBitmap, int x, int y);
	void RenderFrame(FontData& ralewayFont, FontData& inconsolataFont);
	void RenderFrame(char* consoleStr, FontData& ralewayFont, FontData& inconsolataFont);
};