#include "electronics_manager.hpp"
#include <algorithm>

using eip::ElectronicsManager, eip::ElectronicComponent, eip::componentId, eip::ComponentType;

ElectronicsManager::ElectronicsManager()
{
    // load components from database or file here
    this->m_currentId = 1; // for testing
}

ElectronicsManager &ElectronicsManager::instance()
{
    // this is temporary
    // probably get variable value for database ID or something like that
    static ElectronicsManager instance;
    return instance;
}

void ElectronicsManager::addComponent(unique_ptr<ElectronicComponent> component) 
{
    const ComponentType type = component->type();
    ec_vector& components = m_componentMap[type];
    components.push_back(std::move(component));
}

bool ElectronicsManager::removeComponent(componentId id) 
{
    FoundComponent found = _findComponentById(id);
    if (found.found()) {
        const ComponentType type = found.component->type();
        ec_vector &components = m_componentMap[type];
        components.erase(found.iterator);
        return true;
    }

    return false;
}

ElectronicComponent* ElectronicsManager::getComponent(componentId id) 
{
    FoundComponent found = _findComponentById(id);
    return found.found() ? found.component : nullptr;
}

void ElectronicsManager::getAllComponentsByType(ComponentType type, vector<ElectronicComponent *> &outComponents) const
{
    auto it = m_componentMap.find(type);
    if (it != m_componentMap.end()) {
        const ec_vector &components = it->second;
        for (const auto& comp : components) {
            outComponents.push_back(comp.get());
        }
    }
}

void ElectronicsManager::getAllComponents(vector<ElectronicComponent *> &outComponents) const
{
    for (const auto &it : m_componentMap) {
        getAllComponentsByType(it.first, outComponents);
    }
}

ElectronicsManager::FoundComponent ElectronicsManager::_findComponentById(componentId id)
{
    ElectronicsManager::FoundComponent result = {};

    for (auto& [type, components] : m_componentMap) {
        auto it = std::find_if(
            components.begin(),
            components.end(),
            [id](const auto& comp) { return comp->id() == id; }
        );

        if (it != components.end()) {
            result.component = it->get();
            result.container = &components;
            result.iterator = it;
            return result;
        }
    }

    return result; // empty result
}

uint64_t ElectronicsManager::_generateId()
{
    return m_currentId++;
}
