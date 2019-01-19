#ifndef CPP_EXAM_LIBRARY_H
#define CPP_EXAM_LIBRARY_H

#include <cstddef>
#include <memory>
#include <variant>
#include <vector>
#include <iostream>

template<typename T>
class function;

template<typename R, typename... Args>
class function<R(Args...)> {
    class concept {
    public:
        virtual ~concept() = default;

        virtual R operator()(Args &&...args)/* const */= 0;
    };

    template<typename F>
    class model : public concept {
        F f;
    public:
        explicit model(F &&f) : f(std::forward<F>(f)) {}

        R operator()(Args &&... args)/* const */override {
            return f(std::forward<Args>(args)...);
        }
    };

    using word_t = std::byte;

    constexpr static size_t FIXED_SIZE = 16;

    std::array<word_t, FIXED_SIZE> data;
    size_t offset = 0;

    std::shared_ptr<concept> ptr;

    bool small = false;

    void set_pointer_to_data() {
        auto concept_ptr = reinterpret_cast<concept *>(data.data() + offset);
        ptr = std::shared_ptr<concept>(concept_ptr, [](concept *) {});
    }

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
function<R(Args...)>::function(function const &other) : data(other.data), offset(other.offset), small(other.small) {
    if (small) {
        this->set_pointer_to_data();
    } else {
        ptr = other.ptr;
    }
}

template<typename R, typename... Args>
function<R(Args...)>::function(function &&other) noexcept : data(std::move(other.data)), offset(other.offset), small(other.small) {
    if (small) {
        this->set_pointer_to_data();
    } else {
        ptr = std::move(other.ptr);
    }
}

template<typename R, typename... Args>
template<typename F>
function<R(Args...)>::function(F &&f) {
    using concept_t = function::concept;
    using model_t = function::model<F>;
    if (sizeof(std::decay_t<F>) * 2 <= FIXED_SIZE) {
        auto model_ptr = new(reinterpret_cast<model_t *>(data.data())) model_t(std::forward<F>(f));
        offset = model_ptr - reinterpret_cast<model_t *>(data.data());
        ptr = std::shared_ptr<model_t>(model_ptr, [](model_t *) {});

        small = true;
    } else {
        ptr = std::make_shared<model_t>(std::forward<F>(f));

        small = false;
    }
}

template<typename R, typename... Args>
function<R(Args...)>::~function() {

}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(const function &other) {
    auto tmp(other);
    tmp.swap(*this);
    return *this;
}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(function &&other) noexcept {
    auto tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

template<typename R, typename... Args>
void function<R(Args...)>::swap(function &other) noexcept {
    std::swap(data, other.data);
    std::swap(offset, other.offset);
    std::swap(ptr, other.ptr);
    std::swap(small, other.small);

    if (this->small) {
        this->set_pointer_to_data();
    }
    if (other.small) {
        other.set_pointer_to_data();
    }
}

template<typename R, typename... Args>
function<R(Args...)>::operator bool() const noexcept {
    return ptr.get() != nullptr;
}

#endif