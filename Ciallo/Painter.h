#pragma once

#include "CanvasEvent.h"

enum class StrokeUsageFlags
{
	Zero = 0,		// on final image, no fill
	Label = 1 << 0,		// not on final image
	Fill = 1 << 1,		// fill polygon color
	Arrange = 1 << 3,	// insert into arrangement
	Zone = 1 << 4,		// calculate zone
	_entt_enum_as_bitmask
};


class Painter
{
	chrono::duration<float> LastSampleDuration{0.0f};
	
	glm::vec2 LastSampleMousePosPixel{};
public:
	chrono::duration<float> SampleInterval{ 0.01f }; // 10ms
	entt::entity Brush = entt::null;
	glm::vec4 Color = {0.0f, 0.0f, 0.0f, 1.0f};
	float Thickness = 0.001f;
	glm::vec4 FillColor = {0.0f, 0.0f, 0.0f, 1.0f};
	StrokeUsageFlags Usage = StrokeUsageFlags::Zero;

	void OnDragStart(ClickOrDragStart event);
	void OnDragging(Dragging event);
};

