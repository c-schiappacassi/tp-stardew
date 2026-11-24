template <typename Callback>
void Foo::constant_rate_loop(Callback&& func, std::chrono::milliseconds rate, bool& running) {
    using namespace std::chrono;
    auto t1 = steady_clock::now();
    long long it = 0;

    while (running) {
        func(it);

        auto t2 = steady_clock::now();
        auto rest = rate - duration_cast<milliseconds>(t2 - t1);

        if (rest.count() < 0) {
            auto behind = -rest;
            auto lost = behind - (behind % rate);
            t1 += lost;
            it += (lost / rate);
        } else {
            std::this_thread::sleep_for(rest);
        }

        t1 += rate;
        it += 1;
    }
}