#pragma once

class Compute {
    public:
        unsigned int ID;

        Compute(const char* computePath);

        void use();

        ~Compute();
};