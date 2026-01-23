////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "ListenerCollection.h"

namespace DemoRoom
{
	// PredictionUpdated event listener
	class IPredictionUpdateListener abstract
	{
	public:
		enum PredictionUpdateReason
		{
			HolographicSpaceCreateNextFrame,
			HolographicFrameUpdatePrediction,
		};

		virtual void OnPredictionUpdated(
			PredictionUpdateReason reason,
			const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordinateSystem,
			const winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction) = 0;
	};

	////////////////////////////////////////////////////////////////////////////////
	// HolographicScene
	// Maintains a list of our current state of Windows::Perception objects, ensuring the rest of the systems
	// use the same coordinate system, timestamp, etc. 
	class HolographicScene : public System
	{
	public:
		SYS_CTOR(HolographicScene)
		HolographicScene(VfsValue& v, winrt::Windows::Graphics::Holographic::HolographicSpace holographic_space);

		winrt::Windows::Graphics::Holographic::HolographicFrame CurrentFrame() const;
		winrt::Windows::Graphics::Holographic::HolographicSpace HolographicSpace() const;

		winrt::Windows::Perception::Spatial::SpatialCoordinateSystem WorldCoordinateSystem() const;
		winrt::Windows::Perception::PerceptionTimestamp CurrentTimestamp() const;

		void UpdateCurrentPrediction();

		void AddPredictionUpdateListener(IPredictionUpdateListener* listener);
		void RemovePredictionUpdateListener(IPredictionUpdateListener* listener);

	protected:
		bool Initialize(const WorldState& ws) override;
		void Update(double dt) override;
		void Uninitialize() override;

		void OnCurrentStageChanged();
		void OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason);

	private:
		mutable std::shared_mutex m_mutex;
		winrt::Windows::Perception::Spatial::SpatialStageFrameOfReference m_stageFrameOfReference{ nullptr };
		winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference m_stationaryFrameOfReference{ nullptr };
		winrt::event_token m_spatialStageCurrentChanged;

		winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace{ nullptr };
		winrt::Windows::Graphics::Holographic::HolographicFrame m_currentFrame{ nullptr };

		ListenerCollection<IPredictionUpdateListener> m_predictionUpdateListeners;
	};
}
