#pragma once

#include <string>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include "../include/Component.h"

class Entity {
    public:
        Entity() {
        }

        template<typename ComponentType>
        void addComponent(std::shared_ptr<ComponentType> component) {
            std::type_index typeComponent = std::type_index(typeid(ComponentType));
            std::string name = typeComponent.name();
            if(m_components.find(typeComponent) != m_components.end()) {
                std::cerr << "Component : " + name + " has already been added." << std::endl;
                return;
            }
            
            m_components.insert(std::make_pair(typeComponent, component));
        }

        template<typename ComponentType>
        void deleteComponent() {
            std::type_index typeComponent = std::type_index(typeid(ComponentType));
            m_components.erase(typeComponent);
        }

        template<typename ComponentType>
        std::shared_ptr<ComponentType> getComponent() {
            std::type_index typeComponent = std::type_index(typeid(ComponentType));
            if(m_components.find(typeComponent) == m_components.end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ComponentType>(m_components[typeComponent]);
        }

    private:
        std::map<std::type_index, ComponentPtr> m_components;
};

using EntityPtr = std::shared_ptr<Entity>;
