#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template <class Listener>
class Trigger {
    std::vector<std::unique_ptr<Listener>> listeners;

protected:
    template <class F>
    void notify(F&& f) {
        for (auto& l : listeners) f(*l);
    }

public:
    template <class T, class... Args>
    T& add(Args&&... args) {
        static_assert(std::is_base_of_v<Listener, T>, "listener must derive from the trigger's listener type");
        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *owned;
        listeners.push_back(std::move(owned));
        return ref;
    }
};
