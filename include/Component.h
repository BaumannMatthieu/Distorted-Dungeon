#pragma once

class Component {
    public:
        virtual ~Component() {
        
        }
};

using ComponentPtr = std::shared_ptr<Component>;
