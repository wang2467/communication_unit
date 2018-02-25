#include "Queue.h"
#include <thread>
#include <string>

void produce(Queue<char*>& q, int s){
	for (int i = s; i < s+15; i++){
		std::string message_ = "Welcome "+std::to_string(i);
		char* array = new char[message_.size()];
		strcpy(array, message_.c_str());
		printf("produce: %s\n", array);
		q.push(array);
	}
} 

void consume(Queue<char*>& q, int id){
	for(int i = 0; i < 5; i++){
		auto& a = q.pop();
		printf("value: %s, id: %d\n", a, id);
	}
}

int main(){
	Queue<char*> q;
	std::vector<std::thread> consumers;
	for (int i = 0; i < 6; i++){
		std::thread c(std::bind(&consume, std::ref(q), i));
		consumers.push_back(std::move(c));
	}
	std::thread p2(std::bind(produce, std::ref(q), 0));
	std::thread p3(std::bind(produce, std::ref(q), 15));
	p2.join();
	p3.join();
	for (auto& consumer: consumers){
		consumer.join();
	}

	char* src = new char[7];
	std::string m = "Welcome";
	strcpy(src, m.c_str());
	char src2[20] = "jiancheng";

	strncat(src, src2, 9);
	printf("%s\n", src);
}