#include "ParasolComponent.h"

ParasolComponent::ParasolComponent() : isOpen(false), openTimer(0.0f), openDuration(0.5f) {
    // Initialize parasol component
}

ParasolComponent::~ParasolComponent() {
    // Cleanup
}

void ParasolComponent::Update(float delta, Player* player) {
    if (isOpen) {
        openTimer += delta;
        if (openTimer >= openDuration) {
            Close();  // Auto-close after duration
        }
    }
}

void ParasolComponent::Render() {
    // In a real implementation, this would render the parasol sprite
    // For now, just a placeholder
    if (isOpen) {
        LOG("Rendering open parasol");
    }
}

void ParasolComponent::Open() {
    isOpen = true;
    openTimer = 0.0f;
}

void ParasolComponent::Close() {
    isOpen = false;
    openTimer = 0.0f;
}