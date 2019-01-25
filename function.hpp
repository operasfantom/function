#ifndef CPP_EXAM_LIBRARY_H
#define CPP_EXAM_LIBRARY_H

#include <cstddef>
#include <memory>
#include <vector>
#include <iostream>
#include <type_traits>

template<typename T>
class function;

template<typename R, typename... Args>
class function<R(Args...)> {
    class concept {
    public:
        virtual ~concept() = default;

        virtual R operator()(Args &&...args)/* const */= 0;

        virtual concept *copy(std::byte *) = 0;

        virtual concept *move(std::byte *) = 0;
    };

    template<typename F>
    class model : public concept {
        F f;
    public:
        explicit model(F const &f) : f(f) {}

        explicit model(F &&f) : f(std::move(f)) {}

        R operator()(Args &&... args)/* const */override;

        concept *copy(std::byte *destination) override;

        concept *move(std::byte *pointer) override;
    };


    /*template<typename F>
    static std::enable_if_t<!std::is_copy_constructible_v<F>, concept *> copy_model(F const &f, std::byte *destination) {
        return nullptr;
    }*/

    template<typename F>
    static std::enable_if_t<std::is_copy_constructible_v<F>, concept *> copy_model(F const &f, std::byte *destination) {
        return static_cast<concept *>(::new(reinterpret_cast<model<F> *>(destination)) model<F>(f));
    }

    /*template<typename F>
    static std::enable_if_t<!std::is_move_constructible_v<F>, concept *> move_model(F &&f, std::byte *destination) {
        return nullptr;
    }*/

    template<typename F>
    static std::enable_if_t<std::is_move_constructible_v<F>, concept *> move_model(F &&f, std::byte *destination) {
        return static_cast<concept * >(::new(reinterpret_cast<model<F> *>(destination)) model<F>(std::move(f)));
    }


    constexpr static size_t FIXED_SIZE = 32;

    alignas(FIXED_SIZE * sizeof(std::byte)) std::array<std::byte, FIXED_SIZE> data;
    size_t offset = 0;

    std::shared_ptr<concept> shared_ptr;
    concept *ptr = nullptr;

    void reset_pointer() {
        if (is_small()) {
            if (ptr) {
                ptr = reinterpret_cast<concept *>(data.data() + offset);
            }
        } else {
            ptr = shared_ptr.get();
        }
    }

    bool has_value() const {
        return ptr != nullptr;
    }

    bool is_small() const {
        return shared_ptr == nullptr;
    }

public:
    function() noexcept;

    explicit function(std::nullptr_t) noexcept;

    function(function const &other);

    function(function &&other) noexcept;

    template<typename F>
    function(F f);

    ~function();

    function &operator=(const function &other);

    function &operator=(function &&other) noexcept;

    void swap(function &other) noexcept;

    explicit operator bool() const noexcept;

    R operator()(Args &&... args) const;
};

template<typename R, typename... Args>
template<typename F>
R function<R(Args...)>::model<F>::operator()(Args &&... args) {
    return f(std::forward<Args>(args)...);
}

template<typename R, typename... Args>
template<typename F>
typename function<R(Args...)>::concept *function<R(Args...)>::model<F>::copy(std::byte *destination) {
    return copy_model(f, destination);
}

template<typename R, typename... Args>
template<typename F>
typename function<R(Args...)>::concept *function<R(Args...)>::model<F>::move(std::byte *destination) {
    return move_model(std::move(f), destination);
}

template<typename R, typename... Args>
function<R(Args...)>::function() noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(std::nullptr_t) noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(function const &other) : offset(other.offset) {
    if (other.has_value()) {
        if (other.is_small()) {
            ptr = other.ptr->copy(data.data());
        } else {
            shared_ptr = other.shared_ptr;
            ptr = shared_ptr.get();
        }
    }
}

template<typename R, typename... Args>
function<R(Args...)>::function(function &&other) noexcept : offset(other.offset) {
    if (other.has_value()) {
        if (other.is_small()) {
            ptr = other.ptr->move(data.data());
        } else {
            shared_ptr = std::move(other.shared_ptr);
            ptr = shared_ptr.get();
        }
    }
}

template<typename R, typename... Args>
template<typename F>
function<R(Args...)>::function(F f) {
    using model_t = model<F>;
    if (sizeof(std::decay_t<F>) * 2 <= FIXED_SIZE) {
        ptr = ::new(reinterpret_cast<model_t *>(data.data())) model_t(std::move(f));
        offset = static_cast<size_t>(std::distance(data.data(), reinterpret_cast<std::byte *>(ptr)));
    } else {
        shared_ptr = std::make_shared<model_t>(std::move(f));
        ptr = shared_ptr.get();
    }
}

template<typename R, typename... Args>
function<R(Args...)>::~function() {
    if (has_value() && is_small()) {
        ptr->~concept();
    }
}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(const function &other) {
    if (other.has_value()) {
        auto tmp(other);
        swap(tmp);
    }
    return *this;
}

template<typename R, typename... Args>
function<R(Args...)> &function<R(Args...)>::operator=(function &&other) noexcept {
    if (other.has_value()) {
        auto tmp(std::move(other));
        swap(tmp);
    }
    return *this;
}

template<typename R, typename... Args>
void function<R(Args...)>::swap(function &other) noexcept {
    std::swap(data, other.data);
    std::swap(offset, other.offset);
    std::swap(shared_ptr, other.shared_ptr);
    std::swap(ptr, other.ptr);

    this->reset_pointer();
    other.reset_pointer();
}

template<typename R, typename... Args>
function<R(Args...)>::operator bool() const noexcept {
    return has_value();
}

template<typename R, typename... Args>
R function<R(Args...)>::operator()(Args &&... args) const {
    return ptr->operator()(std::forward<Args>(args)...);
}

#endif