#include "resources.h"

std::string resourceTypeToString(ResourceType type) {
    switch(type) {
        case ResourceType::MINERALS: return "Minerals";
        case ResourceType::ENERGY: return "Energy";
        case ResourceType::POPULATION: return "Population";
        case ResourceType::RESEARCH_POINTS: return "Research Points";
        case ResourceType::FUEL: return "Fuel";
        case ResourceType::DURANIUM: return "Duranium";
        case ResourceType::NEUTRONIUM: return "Neutronium";
        case ResourceType::CORUNDIUM: return "Corundium";
        case ResourceType::TRITANIUM: return "Tritanium";
        case ResourceType::BORONIDE: return "Boronide";
        case ResourceType::MERCASSIUM: return "Mercassium";
        case ResourceType::VENDARITE: return "Vendarite";
        case ResourceType::SORIUM: return "Sorium";
        case ResourceType::URIDIUM: return "Uridium";
        case ResourceType::GALLICITE: return "Gallicite";
        default: return "Unknown";
    }
}

ResourceStorage::ResourceStorage() {
    resources[ResourceType::MINERALS] = 0;
    resources[ResourceType::ENERGY] = 0;
    resources[ResourceType::POPULATION] = 100;
    resources[ResourceType::RESEARCH_POINTS] = 0;
    resources[ResourceType::FUEL] = 1000;
    resources[ResourceType::DURANIUM] = 500;
    resources[ResourceType::NEUTRONIUM] = 200;
    resources[ResourceType::CORUNDIUM] = 100;
    resources[ResourceType::TRITANIUM] = 150;
    resources[ResourceType::BORONIDE] = 50;
    resources[ResourceType::MERCASSIUM] = 50;
    resources[ResourceType::VENDARITE] = 30;
    resources[ResourceType::SORIUM] = 100;
    resources[ResourceType::URIDIUM] = 20;
    resources[ResourceType::GALLICITE] = 40;
    
    productionRates[ResourceType::MINERALS] = 10;
    productionRates[ResourceType::ENERGY] = 50;
    productionRates[ResourceType::RESEARCH_POINTS] = 5;
}

int ResourceStorage::get(ResourceType type) const {
    auto it = resources.find(type);
    return (it != resources.end()) ? it->second : 0;
}

void ResourceStorage::add(ResourceType type, int amount) {
    resources[type] += amount;
}

bool ResourceStorage::consume(ResourceType type, int amount) {
    if (get(type) >= amount) {
        resources[type] -= amount;
        return true;
    }
    return false;
}

void ResourceStorage::produce(int turns) {
    for (const auto& pair : productionRates) {
        add(pair.first, pair.second * turns);
    }
}

bool ResourceStorage::canAfford(const std::map<ResourceType, int>& costs) const {
    for (const auto& cost : costs) {
        if (get(cost.first) < cost.second) {
            return false;
        }
    }
    return true;
}

bool ResourceStorage::payCosts(const std::map<ResourceType, int>& costs) {
    if (!canAfford(costs)) {
        return false;
    }
    for (const auto& cost : costs) {
        consume(cost.first, cost.second);
    }
    return true;
}

ResourceNode::ResourceNode(ResourceType type, int amt, double rate)
    : resourceType(type), amount(amt), extractionRate(rate) {}

int ResourceNode::extract(int capacity) {
    int extractable = std::min(amount, static_cast<int>(capacity * extractionRate));
    amount -= extractable;
    return extractable;
}
