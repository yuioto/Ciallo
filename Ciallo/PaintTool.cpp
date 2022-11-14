﻿#include "pch.hpp"
#include "PaintTool.h"

#include "Stroke.h"
#include "CanvasPanel.h"

void PaintTool::ClickOrDragStart()
{
	auto s = std::make_unique<Stroke>();

	s->Position = { Canvas->MousePosOnDrawing };
	s->Width = {0.001f};
	s->Arrangement = &Canvas->ActiveDrawing->Arrangement;
	s->OnChanged();
	Canvas->ActiveDrawing->Strokes.push_back(std::move(s));
	LastSample = chrono::duration<float, std::milli>::zero();
}

void PaintTool::Dragging()
{
	glm::vec2 delta = glm::abs(glm::vec2(ImGui::GetMouseDragDelta(0)));

	if (DraggingDuration - LastSample > SampleInterval && delta.x+delta.y >= 6.0f)
	{
		auto& s = Canvas->ActiveDrawing->Strokes.back();

		glm::vec2 pos = Canvas->MousePosOnDrawing;
		s->Position.emplace_back(pos);
		s->Width.emplace_back(0.001f);
		s->OnChanged();

		LastSample = DraggingDuration;
	}
}
