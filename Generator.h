#pragma once
#include <coroutine>
#include <stdexcept>
#include <exception>
#include <utility>
#include <string>
#include <format>

template <typename T>
class Generator
{
public:
	class StopIteration;
	class promise_type;
	class iterator;
	Generator() = default;
	explicit Generator(promise_type& p) : coroutineHandle(std::coroutine_handle<promise_type>::from_promise(p)) {}
	Generator(Generator const&) = delete;
	Generator& operator=(Generator const&) = delete;
	Generator(Generator&& other) : coroutineHandle(other.coroutineHandle) {
		other.coroutineHandle = nullptr;
	}
	Generator& operator==(Generator&& other) {
		if (this != std::addressof(other)) {
			coroutineHandle = std::exchange(other.coroutineHandle, nullptr);
		}
		return *this;
	}
	~Generator() {
		if (coroutineHandle)
			coroutineHandle.destroy();
	}
	iterator begin() {
		if (coroutineHandle) {
			coroutineHandle.resume();
			if (coroutineHandle.done()) {
				coroutineHandle.promise().rethrow_if_exception();
				return { nullptr };
			}
		}
		return { coroutineHandle };
	}
	iterator end() {
		return { nullptr };
	}
	auto next() {
		coroutineHandle.resume();
		if (coroutineHandle.done()) {
			throw StopIteration("Out of range");
		}
		return *coroutineHandle.promise().value;
	}
private:
	std::coroutine_handle<promise_type> coroutineHandle{ nullptr };
};



template <typename T>
class Generator<T>::promise_type {
public:
	T const* value;
	std::exception_ptr exceptionPtr;
	auto get_return_object() {
		return Generator<T>{*this};
	}
	auto initial_suspend() noexcept {
		return std::suspend_always{};
	}
	auto final_suspend() noexcept {
		return std::suspend_always{};
	}
	void unhandled_exception() noexcept {
		exceptionPtr = std::current_exception();
	}
	void rethrow_if_exception() {
		if (exceptionPtr)
			std::rethrow_exception(exceptionPtr);
	}
	auto yield_value(T const& v) {
		value = &v;
		return std::suspend_always{};
	}
	void return_void() {}

	template <typename V>
	V&& await_transform(V&& v) {
		return std::forward<V>(v);
	}
};


template <typename T>
class Generator<T>::iterator {
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = ptrdiff_t;
	using value_type = T;
	using reference = T const&;
	using pointer = T const*;
	iterator() = default;
	iterator(nullptr_t) : handle(nullptr) {}
	iterator(std::coroutine_handle<Generator<T>::promise_type> arg) : handle(arg) {}
	iterator& operator++() {
		handle.resume();
		if (handle.done()) {
			std::exchange(handle, {})
				.promise().rethrow_if_exception();
		}
		return *this;
	}
	void operator++(int) {
		auto tempIter{ *this };
		++* this;
		return tempIter;
	}
	bool operator!=(iterator const& right) const {
		return !(*this == right);
	}
	bool operator==(iterator const& right) const {
		return handle == right.handle;
	}
	reference operator*() const {
		return *handle.promise().value;
	}
	pointer operator->() const {
		return std::addressof(handle.promise().value);
	}
private:
	std::coroutine_handle<Generator::promise_type> handle{ nullptr };
};

template <typename T>
class Generator<T>::StopIteration : public std::exception {
public:
	StopIteration(std::string_view message){
		m = std::format("ERROR: {}", message);
	}
	virtual const char* what() const noexcept override {
		return m.c_str();
	}
private:
	std::string m;
};