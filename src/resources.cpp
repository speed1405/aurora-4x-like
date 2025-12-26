#include "resources.h"

#include <algorithm>
#include <cctype>

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

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool resourceTypeFromString(const std::string& s, ResourceType& out) {
    const std::string v = toLower(s);
    if (v == "minerals") { out = ResourceType::MINERALS; return true; }
    if (v == "energy") { out = ResourceType::ENERGY; return true; }
    if (v == "population") { out = ResourceType::POPULATION; return true; }
    if (v == "research points" || v == "research_points" || v == "rp" || v == "research") { out = ResourceType::RESEARCH_POINTS; return true; }
    if (v == "fuel") { out = ResourceType::FUEL; return true; }
    if (v == "duranium") { out = ResourceType::DURANIUM; return true; }
    if (v == "neutronium") { out = ResourceType::NEUTRONIUM; return true; }
    if (v == "corundium") { out = ResourceType::CORUNDIUM; return true; }
    if (v == "tritanium") { out = ResourceType::TRITANIUM; return true; }
    if (v == "boronide") { out = ResourceType::BORONIDE; return true; }
    if (v == "mercassium") { out = ResourceType::MERCASSIUM; return true; }
    if (v == "vendarite") { out = ResourceType::VENDARITE; return true; }
    if (v == "sorium") { out = ResourceType::SORIUM; return true; }
    if (v == "uridium") { out = ResourceType::URIDIUM; return true; }
    if (v == "gallicite") { out = ResourceType::GALLICITE; return true; }
    return false;
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

void ResourceStorage::set(ResourceType type, int amount) {
    resources[type] = amount;
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
