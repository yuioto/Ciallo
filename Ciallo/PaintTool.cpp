﻿#include "pch.hpp"
#include "PaintTool.h"

#include "Stroke.h"
#include "CanvasPanel.h"

void PaintTool::ClickOrDragStart()
{
	auto s = std::make_unique<Stroke>();

	glm::vec2 pos = Canvas->MousePosOnDrawing;
	s->Position = {{pos.x, pos.y}};
	s->Width = {0.001f};
	s->OnChanged();
	Canvas->ActiveDrawing->Strokes.push_back(std::move(s));
	LastSample = chrono::duration<float, std::milli>::zero();
}

void PaintTool::Dragging()
{
	if (DraggingDuration - LastSample > SampleInterval)
	{
		auto& s = Canvas->ActiveDrawing->Strokes.back();

		glm::vec2 pos = Canvas->MousePosOnDrawing;
		s->Position.emplace_back(pos.x, pos.y);
		s->Width.emplace_back(0.001f);
		s->OnChanged();

		LastSample = DraggingDuration;
	}
}