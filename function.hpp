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
    using word_t = std::byte;
    using pointer_t = word_t *;

    class concept {
    public:
        virtual ~concept() = default;

        virtual R operator()(Args &&...args)/* const */= 0;

        virtual pointer_t copy(pointer_t) = 0;

        virtual pointer_t move(pointer_t) = 0;
    };

    template<typename F>
    class model : public concept {
        F f;
    public:
        explicit model(F f) : f(std::forward<F>(f)) {}

        R operator()(Args &&... args)/* const */override;

        pointer_t copy(pointer_t destination) override;

        pointer_t move(pointer_t pointer) override;
    };


    template<typename F>
    static std::enable_if_t<!std::is_copy_constructible_v<F>, pointer_t> copy_model(F const &f, pointer_t destination) {
        return nullptr;
    }

    template<typename F>
    static std::enable_if_t<std::is_copy_constructible_v<F>, pointer_t> copy_model(F const &f, pointer_t destination) {
        return reinterpret_cast<pointer_t >(::new(reinterpret_cast<model<F> *>(destination)) model<F>(f));
    }

    template<typename F>
    static std::enable_if_t<!std::is_move_constructible_v<F>, pointer_t> move_model(F &&f, pointer_t destination) {
        return nullptr;
    }

    template<typename F>
    static std::enable_if_t<std::is_move_constructible_v<F>, pointer_t> move_model(F &&f, pointer_t destination) {
        return reinterpret_cast<pointer_t >(::new(reinterpret_cast<model<F> *>(destination)) model<F>(std::move(f)));
    }


    constexpr static size_t FIXED_SIZE = 32;

    std::array<word_t, FIXED_SIZE> data;
    size_t offset = 0;

    std::shared_ptr<concept> shared_ptr;
    concept *ptr = nullptr;

    bool small = false;

    void set_pointer_to_data() {
        ptr = reinterpret_cast<concept *>(data.data() + offset);
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
template<typename F>
R function<R(Args...)>::model<F>::operator()(Args &&... args) {
    return f(std::forward<Args>(args)...);
}

template<typename R, typename... Args>
template<typename F>
std::byte *function<R(Args...)>::model<F>::copy(std::byte *destination) {
    return copy_model<F>(f, destination);
}

template<typename R, typename... Args>
template<typename F>
std::byte *function<R(Args...)>::model<F>::move(std::byte *destination) {
    return move_model<std::decay_t<F>>(std::move(f), destination);
}

template<typename R, typename... Args>
function<R(Args...)>::function() noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(std::nullptr_t) noexcept {

}

template<typename R, typename... Args>
function<R(Args...)>::function(function const &other) : offset(other.offset), small(other.small) {
    if (small) {
        other.ptr->copy(data.data());
        this->set_pointer_to_data();
    } else {
        shared_ptr = other.shared_ptr;
        ptr = shared_ptr.get();
    }
}

template<typename R, typename... Args>
function<R(Args...)>::function(function &&other) noexcept : offset(other.offset), small(other.small) {
    if (small) {
        other.ptr->move(data.data());
        this->set_pointer_to_data();
    } else {
        shared_ptr = std::move(other.shared_ptr);
        ptr = shared_ptr.get();
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
        ptr = model_ptr;
        /*shared_ptr = std::shared_ptr<model_t>(model_ptr, [](model_t *ptr) {
            ptr->destruct();
        });*/

        small = true;
    } else {
        shared_ptr = std::make_shared<model_t>(std::forward<F>(f));
        ptr = shared_ptr.get();

        small = false;
    }
}

template<typename R, typename... Args>
function<R(Args...)>::~function() {
    if (small) {
        ptr->~concept();
    }
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
    std::swap(shared_ptr, other.shared_ptr);
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
    return ptr != nullptr;
}

#endif