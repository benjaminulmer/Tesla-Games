#include "HealthComponent.h"

HealthComponent::HealthComponent(){}
HealthComponent::HealthComponent(double h) : localHealth(h) {}
HealthComponent::~HealthComponent() {};

double HealthComponent::GetHealth() {
	return localHealth;
}

void HealthComponent::SetHealth(double h) {
	localHealth = h;

	return;
}

void HealthComponent::Tick(float deltaTime){}
