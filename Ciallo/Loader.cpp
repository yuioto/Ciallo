#include "pch.hpp"
#include "Loader.h"

#include <fstream>
#include "Polyline.h"
#include "Canvas.h"
#include "Stroke.h"
#include "BrushManager.h"
#include "StrokeContainer.h"
#include "Painter.h"
#include "ArrangementManager.h"
#include "TimelineManager.h"

void Loader::LoadCsv(const std::filesystem::path& filePath, float targetRadius)
{
	// Warning: memory leak! (not trying to remove unused stroke)
	R.ctx().get<StrokeContainer>().StrokeEs.clear();
	entt::entity drawingE = R.ctx().get<TimelineManager>().GetCurrentDrawing();
	if (drawingE == entt::null) return;
	auto& arm = R.get<ArrangementManager>(drawingE);
	arm.Arrangement.clear();

	std::ifstream file(filePath);
	std::string line;
	file.exceptions(std::ifstream::badbit);

	Geom::Polyline curve, allPoints;
	std::vector<Geom::Polyline> curves;
	std::vector<std::vector<float>> pressures; // pen pressures get from gpencil
	std::vector<float> pressure;
	float maxPressure = 0.0f;

	int nStroke = 0, nVertex = 0;

	while (std::getline(file, line))
	{
		if (line.empty())
		{
			++nStroke;
			curves.push_back(std::move(curve));
			pressures.push_back(std::move(pressure));
			curve = Geom::Polyline{};
			pressure.clear();
			continue;
		}

		++nVertex;
		std::vector<float> values;
		for (auto value : views::split(line, ','))
		{
			std::string s(value.begin(), value.end());
			values.push_back(std::stof(s));
		}
		curve.push_back(values[0], values[1]);
		allPoints.push_back(values[0], values[1]);
		pressure.push_back(values[2]);
		if (values[2] >= maxPressure) maxPressure = values[2];
	}

	spdlog::info("Number of strokes: {}", nStroke);
	spdlog::info("Number of vertices: {}", nVertex);

	glm::vec2 boundSize = allPoints.BoundingBox()[1] - allPoints.BoundingBox()[0];
	auto& canvas = R.ctx().get<Canvas>();
	glm::vec2 factorXY = boundSize / canvas.Viewport.GetSize();
	float factor = 1.0f / glm::max(factorXY.x, factorXY.y);
	factor *= 0.8f;
	glm::vec2 mid = (allPoints.BoundingBox()[1] + allPoints.BoundingBox()[0]) / 2.0f;
	for (int i = 0; i < curves.size(); ++i)
	{
		auto& c = curves[i];
		c = c.Scale({factor, factor}, mid);
		c = c.Translate(-mid + canvas.Viewport.GetSize() / 2.0f);
		auto& offset = pressures[i];
		for (float& t : offset) t = -(1.0f - t/maxPressure) * targetRadius;

		entt::entity strokeE = R.create();
		R.emplace<StrokeUsageFlags>(strokeE, StrokeUsageFlags::Final | StrokeUsageFlags::Arrange);
		R.get<StrokeContainer>(drawingE).StrokeEs.push_back(strokeE);
		auto& stroke = R.emplace<Stroke>(strokeE);
		stroke.Position = c;
		stroke.RadiusOffset = offset;
		stroke.Radius = targetRadius;

		// I intented to use create a event system, but I'm lazy.
		stroke.BrushE = R.ctx().get<BrushManager>().Brushes[2];
		stroke.UpdateBuffers();

		arm.AddOrUpdate(strokeE);
	}
}

void Loader::SaveCsv(const std::filesystem::path& filePath)
{
	std::ofstream file(filePath);
	
}

void Loader::LoadAnimation(const std::filesystem::path& filePath)
{
	// Get transformation parameters
	const auto& entry = *std::filesystem::directory_iterator(filePath)++;
	std::ifstream file(entry.path());
	file.exceptions(std::ifstream::badbit);

	Geom::Polyline curve, allPoints;
	std::vector<Geom::Polyline> curves;
	std::vector<std::vector<float>> pressures;
	std::vector<float> pressure;
	float maxPressure = 0.0f;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
		{
			curves.push_back(std::move(curve));
			pressures.push_back(std::move(pressure));
			curve = Geom::Polyline{};
			pressure.clear();
			continue;
		}

		std::vector<float> values;
		for (auto value : views::split(line, ','))
		{
			std::string s(value.begin(), value.end());
			values.push_back(std::stof(s));
		}
		curve.push_back(values[0], values[1]);
		allPoints.push_back(values[0], values[1]);
		pressure.push_back(values[2]);
		if (values[2] >= maxPressure) maxPressure = values[2];
	}

	glm::vec2 boundSize = allPoints.BoundingBox()[1] - allPoints.BoundingBox()[0];
	auto& canvas = R.ctx().get<Canvas>();
	glm::vec2 factorXY = boundSize / canvas.Viewport.GetSize();
	float scaleFactor = 1.0f / glm::max(factorXY.x, factorXY.y);
	scaleFactor *= 0.8f;
	glm::vec2 mid = (allPoints.BoundingBox()[1] + allPoints.BoundingBox()[0]) / 2.0f;

	// Transformation paramers
	glm::vec2 scale = { scaleFactor, scaleFactor };
	glm::vec2 pivot = mid;
	glm::vec2 translate = -mid + canvas.Viewport.GetSize() / 2.0f;

	// Import files
	R.ctx().get<TimelineManager>().Clear();

	for (const auto& entry : std::filesystem::directory_iterator(filePath))
	{
		curves.clear();
		pressures.clear();
		if (entry.path().extension() != ".csv")
			continue;
		std::ifstream file(entry.path());
		// filename without extension is the frame number
		auto frameNumberString = entry.path().filename().replace_extension().string();
		int frameNumber = std::stoi(frameNumberString);
		entt::entity drawingE = R.ctx().get<TimelineManager>().GenKeyFrame(frameNumber);
		auto& arm = R.get<ArrangementManager>(drawingE);

		while (std::getline(file, line))
		{
			if (line.empty())
			{
				curves.push_back(std::move(curve));
				pressures.push_back(std::move(pressure));
				curve = Geom::Polyline{};
				pressure.clear();
				continue;
			}

			std::vector<float> values;
			for (auto value : views::split(line, ','))
			{
				std::string s(value.begin(), value.end());
				values.push_back(std::stof(s));
			}
			curve.push_back(values[0], -values[1]);
			pressure.push_back(values[2]);
		}

		float targetRadius = 0.002f;
		for (int i = 0; i < curves.size(); ++i)
		{
			auto& c = curves[i];
			c = c.Scale({ scaleFactor, scaleFactor }, pivot);
			c = c.Translate(translate);
			auto& offset = pressures[i];
			for (float& t : offset) t = -(1.0f - t / maxPressure) * targetRadius;

			entt::entity strokeE = R.create();
			R.emplace<StrokeUsageFlags>(strokeE, StrokeUsageFlags::Final | StrokeUsageFlags::Arrange);
			R.get<StrokeContainer>(drawingE).StrokeEs.push_back(strokeE);
			auto& stroke = R.emplace<Stroke>(strokeE);
			stroke.Position = c;
			stroke.RadiusOffset = offset;
			stroke.Radius = targetRadius;

			// I intented to use create a event system, but I'm lazy.
			stroke.BrushE = R.ctx().get<BrushManager>().Brushes[2];
			stroke.UpdateBuffers();

			arm.AddOrUpdate(strokeE);
		}
	}
}
