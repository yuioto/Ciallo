﻿#pragma once

#include "Brush.h"
#include "Tool.h"

class PaintTool : public Tool
{
public:
	chrono::duration<float, std::milli> LastSampleDuration{0.0f};
	chrono::duration<float, std::milli> SampleInterval{ 10.f };

	glm::vec2 LastSampleMousePos{};

	Brush* ActiveBrush;

	explicit PaintTool(CanvasPanel* canvas)
		: Tool(canvas)
	{
	}

	void ClickOrDragStart() override;
	void Dragging() override;
};
