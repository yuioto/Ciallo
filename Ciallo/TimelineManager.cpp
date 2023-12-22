#include "pch.hpp"
#include "TimelineManager.h"

#include "imgui_neo_sequencer.h"
#include "StrokeContainer.h"
#include "ArrangementManager.h"

TimelineManager::TimelineManager()
{
    GenKeyFrame(1);
}

void TimelineManager::DrawUI()
{
    ImGui::Begin("Timeline");
    if (ImGui::Button("+", { 50.0, 0.0 })) {
        if (GetCurrentDrawing() == entt::null) {
            GenKeyFrame(CurrentFrame);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("-", { 50.0, 0.0 })) {
		RemoveKeyFrame(CurrentFrame);
    }
    if (ImGui::BeginNeoSequencer("Sequencer", &CurrentFrame, &StartFrame, &EndFrame)) {
        
        ImGui::SameLine();
        if (ImGui::BeginNeoTimeline("Timline", KeyFrames)) {
            
            ImGui::EndNeoTimeLine();
        }
        ImGui::EndNeoSequencer();
    }
    ImGui::End();
}

entt::entity TimelineManager::GetRenderDrawing()
{

    if (entt::entity e = GetCurrentDrawing(); e != entt::null) {
        return e;
    }
    else {
        return GetPreviousDrawing();
    }
}

entt::entity TimelineManager::GetPreviousDrawing()
{
    int index = -1;
    int i = 0;
    int minValue = std::numeric_limits<int>::max();
    for (int keyNum : KeyFrames) {
        if (keyNum < CurrentFrame) {
            int distance = CurrentFrame - keyNum;
            if (distance < minValue) {
                minValue = distance;
                index = i;
            }
        }
        i++;
    }
    if (index == -1) return entt::null;
    return DrawingEs[index];
}

entt::entity TimelineManager::GetCurrentDrawing()
{
    return GetDrawing(CurrentFrame);
}

entt::entity TimelineManager::GetDrawing(int keyNumber)
{
    auto it = std::find(KeyFrames.begin(), KeyFrames.end(), keyNumber);
	if (it != KeyFrames.end()) {
		return DrawingEs[std::distance(KeyFrames.begin(), it)];
	}
	else {
		return entt::null;
	}
}

void TimelineManager::RemoveKeyFrame(int keyNumber)
{
	auto it = std::find(KeyFrames.begin(), KeyFrames.end(), keyNumber);
    if (it != KeyFrames.end()) {
        auto index = std::distance(KeyFrames.begin(), it);
        auto e = DrawingEs[index];
        R.destroy(e);
		DrawingEs.erase(DrawingEs.begin() + std::distance(KeyFrames.begin(), it));
        KeyFrames.erase(it);
    }
}

entt::entity TimelineManager::GenKeyFrame(int keyNumber)
{
    KeyFrames.push_back(keyNumber);
	entt::entity drawingE = R.create();
	R.emplace<StrokeContainer>(drawingE);
	R.emplace<ArrangementManager>(drawingE);
	DrawingEs.push_back(drawingE);
	return drawingE;
}