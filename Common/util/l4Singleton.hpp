#pragma once

template <typename T>
class L4Singleton
{
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
};