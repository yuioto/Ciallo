﻿#include "pch.hpp"
#include "Drawing.h"

#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

Drawing::Drawing()
{
	GenRenderTarget();
}

Drawing::~Drawing()
{
	DeleteRenderTarget();
}

glm::ivec2 Drawing::GetSizeInPixel() const
{
	float width = (LowerRight.x - UpperLeft.x) * Dpi / 0.0254f;
	float height = (LowerRight.y - UpperLeft.y) * Dpi / 0.0254f;
	return {static_cast<int>(glm::round(width)), static_cast<int>(glm::round(height))};
}

glm::vec2 Drawing::GetSizeInPixelFloat() const
{
	float width = (LowerRight.x - UpperLeft.x) * Dpi / 0.0254f;
	float height = (LowerRight.y - UpperLeft.y) * Dpi / 0.0254f;
	return {width, height};
}

glm::vec2 Drawing::GetWorldSize() const
{
	return LowerRight - UpperLeft;
}

void Drawing::GenRenderTarget()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &Texture);
	glTextureParameteri(Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// This is required on WebGL for non power-of-two textures
	glTextureParameteri(Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	glCreateFramebuffers(1, &FrameBuffer);
	glNamedFramebufferTexture(FrameBuffer, GL_COLOR_ATTACHMENT0, Texture, 0);
}

void Drawing::AllocateRenderStorage()
{
	int width = GetSizeInPixel().x;
	int height = GetSizeInPixel().y;

	glTextureStorage2D(Texture, 1, GL_RGBA8, width, height);

	if (glCheckNamedFramebufferStatus(FrameBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Framebuffer incomplete");
	}
}

void Drawing::DeleteRenderTarget()
{
	glDeleteTextures(1, &Texture);
	glDeleteFramebuffers(1, &FrameBuffer);
}

glm::mat4 Drawing::GetViewProjMatrix() const
{
	return glm::ortho(UpperLeft.x, LowerRight.x, UpperLeft.y, LowerRight.y);
}
