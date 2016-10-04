#pragma once

#include <memory>
#include <vector>

#include "../include/Entity.h"

#include "../include/ColliderSystem.hpp"
#include "../include/Dunjeon.hpp"

template<typename SystemType>
class System : public SystemType {
    public:
        void run(std::vector<EntityPtr>& entitys) {
            SystemType::run(entitys);
        }

        template<typename LastComponentType>
        static bool findComponent(EntityPtr entity, std::vector<ComponentPtr>& components) {
            std::shared_ptr<LastComponentType> component = entity->getComponent<LastComponentType>();
            if(component != nullptr) {         
                components.push_back(component);
                return true;
            } 
	        components.clear();
            return false;
        }

        template<typename FirstComponentType, typename SecondComponentType, typename... OtherComponentsType>    
        static bool findComponent(EntityPtr entity, std::vector<ComponentPtr>& components) {
            std::shared_ptr<FirstComponentType> component = entity->getComponent<FirstComponentType>();
            if(component != nullptr) {         
                components.push_back(component);
                return findComponent<SecondComponentType, OtherComponentsType...>(entity, components);
            }
	        components.clear();
            return false;
        }
};

template<typename SystemType>
using SystemPtr = std::shared_ptr<System<SystemType>>;
