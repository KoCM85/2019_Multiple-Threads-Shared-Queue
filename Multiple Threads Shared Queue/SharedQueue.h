#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

#include <condition_variable>
#include <mutex>
#include <memory>
#include <shared_mutex>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <boost/circular_buffer.hpp>

// Thread-safe fixed-size multiple threads shared queue.
template<typename T>
class SharedQueue {
public:
	// Constructor.
	// @param size Queue size.
	SharedQueue(size_t size) : queue(size) {}

	// Constructor.
	// @param size Queue size.
	// @param data std::initializer_list with data.
	// @note copying data from std::initializer_list to boost::circular_buffer. If (data.size() > size) then copying "size" items.
	SharedQueue(size_t size, std::initializer_list<T *> data) : queue(size) {
		size_t data_size = size > data.size() ? data.size() : size;
		auto begin = data.begin();
		auto end = std::next(data.begin(), data_size);

		std::for_each(data.begin(), end, [this](auto & obj) {queue.push_back(std::unique_ptr<T>(obj)); });
	}

	// Copy constructor
	SharedQueue(const SharedQueue &) = delete;

	// Move constructor
	SharedQueue(SharedQueue &&) = delete;

	// Copy assignment
	SharedQueue & operator=(const SharedQueue &) = delete;

	// Move assignment
	SharedQueue & operator=(SharedQueue &&) = delete;

	// Destructor
	~SharedQueue() = default;

	// Gets the number of elements contained in the Queue.
	int count() const {
		std::shared_lock<std::shared_mutex> lock(sha_mut);
		//print();
		return queue.size();
	}

	// Gets SharedQueue capacity
	size_t capacity() const {
		return queue.capacity();
	}

	// Puts the item into the queue.
	// @note if the queue is full then this method blocks until there is the room for the item again.
	void enqueue(T* item) {
		std::unique_lock<std::shared_mutex> lock(sha_mut);

		queue_not_full.wait(lock, [this]() { return queue.size() < queue.capacity(); });
		queue.push_back(std::unique_ptr<T>(item));
		//print();
		lock.unlock();
		queue_not_empty.notify_one();
	}

	// Puts the item into the queue.
	// @param millisecondsTimeout Numbers of milliseconds to wait.
	// @return 'true' if the operation was complited successfully, 'false' if the operation timed out.
	// @note if the queue is full then this method blocks until there is the room for the item again or the operation timed out.
	bool enqueue(T* item, int millisecondsTimeout) {
		std::unique_lock<std::shared_mutex> lock(sha_mut);
		bool status = queue_not_full.wait_for(lock, std::chrono::milliseconds(millisecondsTimeout), [this]() {return queue.size() < queue.capacity(); });
		
		if (status) {
			queue.push_back(std::unique_ptr<T>(item));
			//print();
			lock.unlock();
			queue_not_empty.notify_one();
		}
		else {
			//print();
			lock.unlock();
		}

		return status;
	}

	// Removes and returns the item at the beginning of the Queue.
	// @note if the queue is empty then this method blocks until there is an item again.
	T* dequeue() {
		std::unique_lock<std::shared_mutex> lock(sha_mut);

		queue_not_empty.wait(lock, [this]() {return queue.size() > 0; });
		std::unique_ptr<T> begin_queue = std::move(queue.front());
		queue.pop_front();
		//print();
		lock.unlock();
		queue_not_full.notify_one();

		return begin_queue.release();
	}

	// Removes and returns the item at the beginning of the Queue.
	// @param millisecondsTimeout Numbers of milliseconds to wait.
	// @returns The item at the beginning of the Queue or NULL if the operation timed out.
	// @note if the queue is empty then this method blocks until there is an item again or the operation timed out.
	T* dequeue(int millisecondsTimeout) {
		std::unique_lock<std::shared_mutex> lock(sha_mut);
		bool status = queue_not_empty.wait_for(lock, std::chrono::milliseconds(millisecondsTimeout), [this]() {return queue.size() > 0; });
		std::unique_ptr<T> begin_queue;

		if (status) {
			begin_queue = std::move(queue.front());
			queue.pop_front();
			//print();
			lock.unlock();
			queue_not_full.notify_one();
		}
		else {
			//print();
			lock.unlock();
		}

		return begin_queue.release();
	}

	void print() const {
		std::cout << queue.size() << " - " << queue.capacity() << '\n';
		for (auto && val : queue)
			std::cout << *val << ' ';
		std::cout << '\n';
	}

	// Declared like friend to take private data (boost::circular_buffer<T> queue).
	template<typename T>
	friend bool operator ==(const SharedQueue<T> & left_obj, const SharedQueue<T> & right_obj);

private:
	mutable std::shared_mutex sha_mut;
	std::condition_variable_any queue_not_full;
	std::condition_variable_any queue_not_empty;
	boost::circular_buffer<std::unique_ptr<T>> queue;
};

// Definition operator == to check equality.
template<typename T>
inline bool operator ==(const SharedQueue<T> & left_obj, const SharedQueue<T> & right_obj) {
	std::shared_lock<std::shared_mutex> lock1(left_obj.sha_mut);
	std::shared_lock<std::shared_mutex> lock2(right_obj.sha_mut);
	return left_obj.queue.size() == right_obj.queue.size() && std::equal(left_obj.queue.begin(), left_obj.queue.end(), right_obj.queue.begin(), [](auto & first, auto & second) {return *first == *second; });
}

#endif //SHAREDQUEUE_H