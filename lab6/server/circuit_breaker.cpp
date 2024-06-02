#include "circuit_breaker.h"

#include <cassert>
#include <string>
#include <iostream>
#include <chrono>

std::ostream& operator<<(std::ostream& os, CircuitBreaker::ServiceState& state) {
    os << "Curcuit breaker: " << state.base_url << " ";
    switch (state.state) {
    case CircuitBreaker::Open:
        os << "Open";
        break;
    case CircuitBreaker::Close:
        os << "Close, " << state.fail_counter.size() << " fail";
        break;
    case CircuitBreaker::Semiopen:
        os << "Semiopen, " << state.success_counter << " success";
        break;
    }
    return os;
}

void CircuitBreaker::checkResponse(const std::string& service, int response) {
    if (response >= 500) {
        failure(service);
    } else {
        success(service);
    }
}

void CircuitBreaker::failure(const std::string& service) {
    if (!_services.contains(service)) {
        _services[service] = ServiceState{.base_url = service};
    }
    ServiceState& state = _services[service];

    assert(state.state != Open);

    if (state.state == Close) {
        time_point_t cur = clock::now();
        state.fail_counter.push_back(cur);
        while (cur - state.fail_counter.front() > _fail_counter_window) {
            state.fail_counter.pop_front();
        }

        if (static_cast<int>(state.fail_counter.size()) > _fail_threshold) {
            state.open();
        }
    } else if (state.state == Semiopen) {
        state.open();
    }
}

void CircuitBreaker::success(const std::string& service) {
    if (!_services.contains(service)) {
        _services[service] = ServiceState{.base_url = service};
    }
    ServiceState& state = _services[service];

    assert(state.state != Open);
    
    if (state.state == Semiopen) {
        ++state.success_counter;
        if (state.success_counter > _success_threshold) {
            state.close();
        }
    }
}

bool CircuitBreaker::status(const std::string& service) {
    if (!_services.contains(service)) {
        _services[service] = ServiceState{.base_url = service};
    }
    ServiceState& state = _services[service];

    if (state.state == Open) {
        time_point_t cur = clock::now();
        if (cur - state.open_start > _timeout) {
            state.semiopen();
        }
    }
    std::cout << state << std::endl;
    return state.state != Open;
}

void CircuitBreaker::ServiceState::open() {
    std::cout << "Circuit breaker for " << base_url << " open" << std::endl;
    state = Open;
    open_start = clock::now();
    fail_counter.clear();
    success_counter = 0;
}

void CircuitBreaker::ServiceState::close() {
    std::cout << "Circuit breaker for " << base_url << " close" << std::endl;
    state = Close;
    fail_counter.clear();
    success_counter = 0;
}

void CircuitBreaker::ServiceState::semiopen() {
    std::cout << "Circuit breaker for " << base_url << " semiopen" << std::endl;
    state = Semiopen;
    fail_counter.clear();
    success_counter = 0;
}
