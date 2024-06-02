#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>

struct Service {
    std::string name;
    std::string base_url;
};

class CircuitBreaker {
public:
    enum State {
        Open,
        Close,
        Semiopen
    };

    using duration_t = std::chrono::duration<double>;
    using clock = std::chrono::steady_clock;
    using time_point_t = std::chrono::time_point<clock, duration_t>;

    CircuitBreaker(int fail_threshold, int success_threshold, duration_t timeout, duration_t fail_counter_window)
        : _fail_threshold{fail_threshold}, _success_threshold{success_threshold}, _timeout{timeout}, _fail_counter_window{fail_counter_window} {}

    void checkResponse(const std::string& service, int response);
    void failure(const std::string& service);
    void success(const std::string& service);
    bool status(const std::string& service);

private:
    struct ServiceState {
        std::string base_url;
        State state = Close;
        std::deque<time_point_t> fail_counter{};
        int success_counter = 0;
        time_point_t open_start{};

        void open();
        void close();
        void semiopen();
    };

    friend std::ostream& operator<<(std::ostream& os, ServiceState& state);

    std::unordered_map<std::string, ServiceState> _services;
    const int _fail_threshold;
    const int _success_threshold;
    const duration_t _timeout;
    const duration_t _fail_counter_window;
};