#ifndef TEST_SHAREDQUEUE_H
#define TEST_SHAREDQUEUE_H

#define BOOST_TEST_MODULE SharedQueue_test

#include "SharedQueue.h"

#include <chrono>
#include <thread>
#include <vector>
#include <numeric>
#include <atomic>

#include <boost/test/unit_test.hpp>


//------------------------Functionality tests------------------------
BOOST_AUTO_TEST_SUITE(Functionality_tests)

BOOST_AUTO_TEST_CASE(constructors_count_capacity_test) {

	std::cout << "All tests tested with SharedQueue<int> \n";

#define negative -12
#define zero 0
#define	positive 12

	// 1.1
	{
		bool checker = false;

		try {
			SharedQueue<int> shaqueue1(negative);
		}
		catch (...) {
			checker = true;
		}
		BOOST_TEST(checker);
	}
	
	// 1.2
	{
		SharedQueue<int> shaqueue1(zero);

		BOOST_TEST(shaqueue1.count() == 0);
		BOOST_TEST(shaqueue1.capacity() == 0);
	}

	// 1.3
	{
		SharedQueue<int> shaqueue1(positive);

		BOOST_TEST(shaqueue1.count() == 0);
		BOOST_TEST(shaqueue1.capacity() == positive);
	}


	// 2.1
	{
		std::unique_ptr<int> ptr1(std::make_unique<int>(54));
		std::unique_ptr<int> ptr2(std::make_unique<int>(-32));
		std::unique_ptr<int> ptr3(std::make_unique<int>(0));
		bool checker = false;

		try {
			SharedQueue<int> shaqueue1(negative, { ptr1.get(), ptr2.get(), ptr3.get() });

			ptr1.release();
			ptr2.release();
			ptr3.release();
		}
		catch (...) {
			checker = true;
		}
		BOOST_TEST(checker);
	}

	// 2.2
	{
		std::unique_ptr<int> ptr1(std::make_unique<int>(54));
		std::unique_ptr<int> ptr2(std::make_unique<int>(-32));
		std::unique_ptr<int> ptr3(std::make_unique<int>(0));
		SharedQueue<int> shaqueue1(zero, { ptr1.get(), ptr2.get(), ptr3.get() });

		BOOST_TEST(shaqueue1.count() == 0);
		BOOST_TEST(shaqueue1.capacity() == 0);
	}

	// 2.3
	{
		std::unique_ptr<int> ptr1(std::make_unique<int>(54));
		std::unique_ptr<int> ptr2(std::make_unique<int>(-32));
		std::unique_ptr<int> ptr3(std::make_unique<int>(0));
		SharedQueue<int> shaqueue1(positive, { ptr1.get(), ptr2.get(), ptr3.get() });

		ptr1.release();
		ptr2.release();
		ptr3.release();

		BOOST_TEST(shaqueue1.count() == 3);
		BOOST_TEST(shaqueue1.capacity() == positive);
	}
#undef negative
#undef zero
#undef positive
}

BOOST_AUTO_TEST_CASE(enqueue_test) {
#define sleep_thread 50
#define vector_size 7

	std::vector<std::thread> threads;
	SharedQueue<int> shaqueue1(5);
	bool checker1;
	bool checker2;

	threads.reserve(vector_size);

	// 1
	for (int counter = 0; counter < 5; ++counter)
		shaqueue1.enqueue(new int(counter));
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(0), new int(1), new int(2), new int(3), new int(4) });
	BOOST_TEST(checker1);

	// 2
	threads.emplace_back([&]() {shaqueue1.enqueue(new int(10)); });
	threads.emplace_back([&]() {shaqueue1.enqueue(new int(11)); });
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(0), new int(1), new int(2), new int(3), new int(4) });
	BOOST_TEST(checker1);

	// 3
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(1), new int(2), new int(3), new int(4), new int(10) });
	checker2 = shaqueue1 == SharedQueue<int>(5, { new int(1), new int(2), new int(3), new int(4), new int(11) });
	BOOST_TEST((checker1 || checker2));

	// 4
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(2), new int(3), new int(4), new int(10), new int(11) });
	checker2 = shaqueue1 == SharedQueue<int>(5, { new int(2), new int(3), new int(4), new int(11), new int(10) });
	BOOST_TEST((checker1 || checker2));

	// 5
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::unique_ptr<int>(shaqueue1.dequeue());
	shaqueue1.enqueue(new int(24), 24);
	shaqueue1.enqueue(new int(2000), 2000);
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(4), new int(10), new int(11), new int(24), new int(2000) });
	checker2 = shaqueue1 == SharedQueue<int>(5, { new int(4), new int(11), new int(10), new int(24), new int(2000) });
	BOOST_TEST((checker1 || checker2));

	// 6
	threads.emplace_back([&]() {shaqueue1.enqueue(new int(1), 1); });
	threads.emplace_back([&]() {shaqueue1.enqueue(new int(10000), 10000); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(11), new int(24), new int(2000), new int(10000) });
	checker2 = shaqueue1 == SharedQueue<int>(5, { new int(10), new int(24), new int(2000), new int(10000) });
	BOOST_TEST((checker1 || checker2));

	// 7
	BOOST_TEST(shaqueue1.enqueue(new int(7666), 7666));

	// 8
	threads.emplace_back([&]() {checker1 = shaqueue1.enqueue(new int(2), 2); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	BOOST_TEST(!checker1);

	// 9
	threads.emplace_back([&]() {checker1 = shaqueue1.enqueue(new int(9996), 9996); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	BOOST_TEST(checker1);

	// 10
	checker1 = shaqueue1 == SharedQueue<int>(5, { new int(24), new int(2000), new int(10000), new int(7666), new int(9996) });
	BOOST_TEST(checker1);

	for (auto && thread : threads)
		thread.join();

#undef sleep_thread
#undef vector_size
}

BOOST_AUTO_TEST_CASE(dequeue_test) {
#define sleep_thread 50
#define vector_size 8

	std::vector<std::thread> threads;
	SharedQueue<int> shaqueue1(5);
	bool checker;

	threads.reserve(vector_size);

	// 1
	threads.emplace_back([&]() {std::unique_ptr<int>(shaqueue1.dequeue(5)); });
	threads.emplace_back([&]() {std::unique_ptr<int>(shaqueue1.dequeue(23)); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	shaqueue1.enqueue(new int(1));
	shaqueue1.enqueue(new int(2));
	checker = shaqueue1 == SharedQueue<int>(7, { new int(1), new int(2) });
	BOOST_TEST(checker);

	// 2
	std::unique_ptr<int>(shaqueue1.dequeue());
	std::unique_ptr<int>(shaqueue1.dequeue(23));
	checker = shaqueue1 == SharedQueue<int>(7);
	BOOST_TEST(checker);

	// 3
	threads.emplace_back([&]() {std::unique_ptr<int>(shaqueue1.dequeue()); });
	threads.emplace_back([&]() {std::unique_ptr<int>(shaqueue1.dequeue(9000)); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	shaqueue1.enqueue(new int(161));
	shaqueue1.enqueue(new int(290));
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	checker = shaqueue1 == SharedQueue<int>(7);
	BOOST_TEST(checker);

	// 4
	threads.emplace_back([&]() {checker = std::unique_ptr<int>(shaqueue1.dequeue(2)).get(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	BOOST_TEST(!checker);

	// 5
	threads.emplace_back([&]() {checker = std::unique_ptr<int>(shaqueue1.dequeue()).get(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	shaqueue1.enqueue(new int(909));
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	BOOST_TEST(checker);

	// 6
	threads.emplace_back([&]() {checker = std::unique_ptr<int>(shaqueue1.dequeue(9000)).get(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	shaqueue1.enqueue(new int(401));
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_thread));
	BOOST_TEST(checker);

	for (auto && thread : threads)
		thread.join();

#undef sleep_thread
#undef vector_size
}

BOOST_AUTO_TEST_CASE(operators_test) {
	SharedQueue<int> shaqueue1(2);
	SharedQueue<int> shaqueue2(2);
	SharedQueue<int> shaqueue3(4, { new int(765), new int(908) });
	bool checker;

	// 1
	checker = shaqueue1 == shaqueue2;
	BOOST_TEST(checker);

	// 2
	shaqueue2.enqueue(new int(765));
	checker = shaqueue1 == shaqueue2;
	BOOST_TEST(!checker);

	// 3
	shaqueue1.enqueue(new int(765));
	checker = shaqueue1 == shaqueue2;
	BOOST_TEST(checker);

	// 4
	shaqueue1.enqueue(new int(908));
	checker = shaqueue1 == shaqueue2;
	BOOST_TEST(!checker);

	// 5
	checker = shaqueue1 == shaqueue3;
	BOOST_TEST(checker);

	// 6
	shaqueue1.enqueue(new int(303), 3);
	shaqueue1.enqueue(new int(404), 3);
	shaqueue3.enqueue(new int(303), 3);
	shaqueue3.enqueue(new int(404), 3);
	checker = shaqueue1 == shaqueue3;
	BOOST_TEST(!checker);
}

BOOST_AUTO_TEST_SUITE_END() //Functionality_tests



//-------------------------Performance tests-------------------------
BOOST_AUTO_TEST_SUITE(Performance_tests)

BOOST_AUTO_TEST_CASE(test) {

	std::vector<double> latencies;
	std::vector<std::thread> threads;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> end;
	std::chrono::duration<double, std::micro> latency;
	double average_latency;

	latencies.reserve(20);
	threads.reserve(2);

	for (size_t counter = 0; counter < 20; ++counter) {
		SharedQueue<int> shaqueue1(4);
		std::unique_ptr<int> value(new int(203));
		
		auto Producer = [&]() {
			start = std::chrono::steady_clock::now();
			shaqueue1.enqueue(value.get());
		};
		auto Consumer = [&]() {
			std::unique_ptr<int>(shaqueue1.dequeue());
			end = std::chrono::steady_clock::now();
		};

		threads.emplace_back(Producer);
		threads.emplace_back(Consumer);

		for (auto && thread : threads)
			thread.join();

		value.release();
		latency = end - start;
		latencies.emplace_back(latency.count());
		threads.clear();
	}

	std::sort(latencies.begin(), latencies.end());
	average_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

	std::cout << "-------------------------Performance tests-------------------------\n";
	std::cout << "minimum latency: ~" << *latencies.begin() << " microseconds \n";
	std::cout << "average latency: ~" << average_latency << " microseconds \n";
	std::cout << "maximum latency: ~" << *(latencies.end() - 1) << " microseconds \n";
}

BOOST_AUTO_TEST_SUITE_END() //Performance_tests



//---------------------------Stress tests----------------------------
BOOST_AUTO_TEST_SUITE(Stress_tests)

// stress function
void stress_test(const size_t amount_Producers, const size_t amount_Consumers, const size_t size_SharedQueue, const int millisec_wait) {
	std::vector<std::thread> Producer_threads;
	std::vector<std::thread> Consumer_threads;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::atomic<size_t> amount_elements(0);

	Producer_threads.reserve(amount_Producers);
	Consumer_threads.reserve(amount_Consumers);

	SharedQueue<int> shaqueue1(size_SharedQueue);

	auto Producer = [&]() {
		std::chrono::duration<double> time;

		do {
			time = std::chrono::steady_clock::now() - start;
			if (shaqueue1.enqueue(new int(1), millisec_wait))
				++amount_elements;
		} while (time.count() < 1.0);
	};
	auto Consumer = [&]() {
		std::chrono::duration<double> time;

		do {
			time = std::chrono::steady_clock::now() - start;
			std::unique_ptr<int>(shaqueue1.dequeue(millisec_wait));
		} while (time.count() < 1.0);
	};

	start = std::chrono::steady_clock::now();
	for (size_t counter = 0; counter < amount_Producers; ++counter) {
		Producer_threads.emplace_back(Producer);
	}
	for (size_t counter = 0; counter < amount_Consumers; ++counter) {
		Consumer_threads.emplace_back(Consumer);
	}

	for (auto && thread : Producer_threads)
		thread.join();

	for (auto && thread : Consumer_threads)
		thread.join();

	std::cout << "----------------------------Stress tests---------------------------\n";
	std::cout << "Producers " << amount_Producers << ", Consumers " << amount_Consumers << ": \n";
	std::cout << "Items inserted: " << amount_elements << '\n';
	std::cout << "Items removed:  " << amount_elements - shaqueue1.count() << '\n';
	std::cout << "Items left in the SharedQueue: " << shaqueue1.count() << '\n';
	std::cout << "SharedQueue capacity: " << shaqueue1.capacity() << '\n';
}

#define size_SharedQueue 1000
#define millisec_wait 1

BOOST_AUTO_TEST_CASE(test_1) {

	stress_test(50, 100, size_SharedQueue, millisec_wait);
}

BOOST_AUTO_TEST_CASE(test_2) {

	stress_test(100, 50, size_SharedQueue, millisec_wait);
}

#undef size_SharedQueue
#undef millisec_wait

BOOST_AUTO_TEST_SUITE_END() //Stress_tests

#endif //TEST_SHAREDQUEUE_H