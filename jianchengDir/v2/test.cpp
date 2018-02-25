#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <deque>
#include <condition_variable>

std::deque<int> dq;
std::mutex mt;
std::condition_variable cond;

void f1(){
	int ct = 10;
	while (ct > 0){
		// std::unique_lock<std::mutex> locker(mt);
		std::lock_guard<std::mutex> locker(mt);
		dq.push_back(ct);
		// locker.unlock();
		cond.notify_one();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		ct--;
	}
}

void f2(){
	int a = 0;
	while (a != 1){
		std::unique_lock<std::mutex> locker(mt);
		cond.wait(locker, [](){
			return !(dq.empty());
		});
		a = dq.back();
		dq.pop_back();
		locker.unlock();
		std::cout << "F2 get value: " << a << std::endl;
	}
}

int main(){
	std::thread t1(f1);
	std::thread t2(f2);
	t1.join();
	t2.join();
}
// static int a = 0;

// struct Counter{
// 	int value;
// 	Counter():value(0){}
// 	void increment(){++value;}
// };

// struct ConcurrentCounter{
// 	std::mutex mt;
// 	Counter c;

// 	void increment(){
// 		std::lock_guard<std::mutex> guard(mt);
// 		c.increment();
// 	}
// };

// void hello(){
// 	printf("Hello: %d\n", a++);
// }

// int main(){
// 	ConcurrentCounter c;
// 	std::vector<std::thread> ts;
// 	for(int i = 0; i < 5; i++){
// 		ts.push_back(std::thread([&c](){
// 			for(int j = 0; j < 100;j++){
// 				c.increment();
// 			}
// 		}));
// 	}

// 	for (auto& t: ts){
// 		t.join();
// 	}

// 	std::cout << c.c.value << std::endl;

// 	return 0;
// }