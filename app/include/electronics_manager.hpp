#pragma once

#include "electronic_components.hpp"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>

using std::vector, std::unique_ptr, std::map, std::unordered_map, std::uint64_t;

namespace eip {
    class ElectronicsManager {
        using ec_vector = vector<unique_ptr<ElectronicComponent>>;

        public:
        
        ElectronicsManager();

        static ElectronicsManager& instance();

        void addComponent(unique_ptr<ElectronicComponent> component);
        bool removeComponent(componentId id);
        ElectronicComponent* getComponent(componentId id);

        void getAllComponentsByType(ComponentType type, vector<ElectronicComponent*>& outComponents) const;
        void getAllComponents(vector<ElectronicComponent*>& outComponents) const;
        private:

        struct FoundComponent {
            ElectronicComponent* component = nullptr;
            ec_vector* container = nullptr;   // the vector it was found in
            ec_vector::iterator iterator;     // optional: where inside the vector
            bool found() const { return component != nullptr; }
        };

        FoundComponent _findComponentById(componentId id);
        
        uint64_t _generateId();
        unordered_map<ComponentType, ec_vector> m_componentMap;
        uint64_t m_currentId;
    };
};