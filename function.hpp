#ifndef CPP_EXAM_LIBRARY_H
#define CPP_EXAM_LIBRARY_H

#include <cstddef>
#include <memory>


template<typename T>
class function;

template<typename R, typename... Args>
class function<R(Args...)> {
//    template<typename R, typename... Args>
    class concept {
    public:
        virtual R operator()(Args &&...args) const = 0;
    };

    template<typename F/*, typename R, typename... Args*/>
    class model : public concept/*<R, Args...>*/ {
        F f;
    public:
        explicit model(F &&f) : f(std::forward<F>(f)) {}

        R operator()(Args &&... args) const override {
            return f(std::forward<Args>(args)...);
        }
    };

    std::shared_ptr<concept/*<R, Args...>*/> ptr;
public:
    function() noexcept;

    explicit function(std::nullptr_t) noexcept;

    function(function const &other);

    function(function &&other) noexcept;

    template<typename F>
    function(F &&f);

    ~function();

    function &operator=(const function &other);

    function &operator=(function &&other) noexcept;

    void swap(function &other) noexcept;

    explicit operator bool() const noexcept;

    template<typename... Params>
    R operator()(Params &&... params) const {
        return ptr->operator()(std::forward<Params>(params)...);
    }
};

template<typename R, typename... Args>
function<R(Args...)>::function() noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(std::nullptr_t) noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(function const &other) : ptr(other.ptr) {

}

template<typename R, typename... Args>
function<R(Args...)>::function(function &&other) noexcept : ptr(std::move(other.ptr)) {

}

template<typename R, typename... Args>
template<typename F>
function<R(Args...)>::function(F &&f) : ptr(std::make_shared<function::model<F>>(std::forward<F>(f))) {

}

template<typename R, typename... Args>
function<R(Args...)>::~function() {

}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(const function &other) {
    ptr = other.ptr;
    return *this;
}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(function &&other) noexcept {
    ptr = std::move(other.ptr);
    return *this;
}

template<typename R, typename... Args>
void function<R(Args...)>::swap(function &other) noexcept {
    std::swap(ptr, other.ptr);
}

template<typename R, typename... Args>
function<R(Args...)>::operator bool() const noexcept {
    return ptr;
}

#endif