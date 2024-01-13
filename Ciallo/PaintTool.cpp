﻿#include "pch.hpp"
#include "PaintTool.h"

#include "BrushManager.h"
#include "TimelineManager.h"
#include "StrokeContainer.h"
#include "ArrangementManager.h"
#include "TextureManager.h"

PaintTool::PaintTool()
{
	auto& brushM = R.ctx().get<BrushManager>();
	Painter.BrushE = brushM.Brushes[2];
	Painter.Usage = StrokeUsageFlags::Arrange | StrokeUsageFlags::Final;
	Painter.SampleInterval =  chrono::duration<float>(0.01f);
	Painter.MinRadius = 0.5e-3f;
	Painter.MaxRadius = 1e-3f;
}

void PaintTool::OnClickOrDragStart(ClickOrDragStart event)
{
	Painter.OnDragStart(event);
}

void PaintTool::OnDragging(Dragging event)
{
	Painter.OnDragging(event);
}

std::string PaintTool::GetName()
{
	return "Paint";
}

void PaintTool::DrawProperties()
{
	ImGui::CheckboxFlags("Show in final image", reinterpret_cast<unsigned*>(&Painter.Usage),
		                     static_cast<unsigned>(StrokeUsageFlags::Final));

	ImGui::CheckboxFlags("Fill color", reinterpret_cast<unsigned*>(&Painter.Usage),
		                     static_cast<unsigned>(StrokeUsageFlags::Fill));
	Painter.DrawProperties();
	if (ImGui::IsKeyPressed(ImGuiKey_Z)) {
		entt::entity drawingE = R.ctx().get<TimelineManager>().GetCurrentDrawing();
		if (drawingE == entt::null) return;
		auto& strokes = R.get<StrokeContainer>(drawingE).StrokeEs;

		entt::entity strokeE = strokes.back();

		auto& strokeUsage = R.get<StrokeUsageFlags>(strokeE);
		auto currentDrawingE = R.ctx().get<TimelineManager>().GetCurrentDrawing();
		if (currentDrawingE == entt::null) return;
		if (!!(strokeUsage & StrokeUsageFlags::Arrange))
			R.get<ArrangementManager>(currentDrawingE).Remove(strokeE);
		if (!!(strokeUsage & StrokeUsageFlags::Zone))
			R.get<ArrangementManager>(currentDrawingE).RemoveQuery(strokeE);

		strokes.pop_back();
		R.destroy(strokeE);
	}
	int w, h;
	int miplevel = 0;
	glGetTextureLevelParameteriv(TextureManager::Textures[5], miplevel, GL_TEXTURE_WIDTH, &w);
	glGetTextureLevelParameteriv(TextureManager::Textures[5], miplevel, GL_TEXTURE_HEIGHT, &h);

	ImGui::Image(reinterpret_cast<ImTextureID>(TextureManager::Textures[5]), {float(w), float(h)});
}
