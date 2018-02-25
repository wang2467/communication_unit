#include <deque>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class Worker{
public:
	typedef std::function<void ()> job;
	Worker(): exit(false){
		thread = std::unique_ptr<std::thread>(new std::thread(std::bind(&Worker::Entry, this)));
	}

	void addJob(job jt){
		std::lock_guard<std::mutex> guard(mutex_);
		jobs.push_back(jt);
		cond_.notify_one();
	}

	~Worker(){
		std::unique_lock<std::mutex> locker(mutex_);
		exit = true;
		locker.unlock();
		cond_.notify_one();
		thread -> join();
	}

private:
	std::mutex mutex_;
	std::condition_variable cond_;
	std::deque<job> jobs;
	std::unique_ptr<std::thread> thread;
	bool exit;

	void Entry(){
		job j;

		while (true){
			std::unique_lock<std::mutex> locker(mutex_);
			cond_.wait(locker, [&](){
				return exit || !(jobs.empty());
			});
			if (exit){
				return;
			}
			j = jobs.back();
			jobs.pop_back();
			locker.unlock();
			j();
		}		
	}
};

int main(){
	using namespace std;
	{
		Worker w;
		w.addJob([](){
			cout << "Print 1" << endl; 
		});
		w.addJob([](){
			cout << "Print 2" << endl; 
		});
		w.addJob([](){
			cout << "Print 3" << endl; 
		});
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

}
