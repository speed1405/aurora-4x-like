#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>
#include <map>

enum class ResourceType {
    MINERALS,
    ENERGY,
    POPULATION,
    RESEARCH_POINTS,
    FUEL,
    DURANIUM,
    NEUTRONIUM,
    CORUNDIUM,
    TRITANIUM,
    BORONIDE,
    MERCASSIUM,
    VENDARITE,
    SORIUM,
    URIDIUM,
    GALLICITE
};

std::string resourceTypeToString(ResourceType type);
bool resourceTypeFromString(const std::string& s, ResourceType& out);

class ResourceStorage {
private:
    std::map<ResourceType, int> resources;
    std::map<ResourceType, int> productionRates;

public:
    ResourceStorage();
    
    int get(ResourceType type) const;
    void add(ResourceType type, int amount);
    void set(ResourceType type, int amount);
    bool consume(ResourceType type, int amount);
    void produce(int turns = 1);
    bool canAfford(const std::map<ResourceType, int>& costs) const;
    bool payCosts(const std::map<ResourceType, int>& costs);

    const std::map<ResourceType, int>& snapshot() const { return resources; }
};

class ResourceNode {
private:
    ResourceType resourceType;
    int amount;
    double extractionRate;

public:
    ResourceNode(ResourceType type, int amt, double rate = 1.0);
    
    int extract(int capacity);
    ResourceType getType() const { return resourceType; }
    int getAmount() const { return amount; }
};

#endif // RESOURCES_H
