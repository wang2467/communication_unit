#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>

class BoundedBuffer{
public:
	BoundedBuffer(int c):front(0), rear(c-1), count(0), capacity(c){
		buffer = new int[c];
	}

	void deposit(int x){
		std::unique_lock<std::mutex> locker(mutex_);
		cond_.wait(locker, [this](){
			return count != capacity;
		});
		buffer[rear] = x;
		rear = (rear+1) % capacity;
		count++;
		locker.unlock();
		cond_.notify_one();
	}

	int fetch(){
		std::unique_lock<std::mutex> locker(mutex_);
		cond_.wait(locker, [this](){
			return count != 0;
		});
		int x = buffer[front];
		front = (front+1) % capacity;
		count--;
		locker.unlock();
		cond_.notify_one();
		return x;
	}
	
private:
	int front;
	int rear;
	int* buffer;
	int count;
	int capacity;
	std::mutex mutex_;
	std::condition_variable cond_;

};