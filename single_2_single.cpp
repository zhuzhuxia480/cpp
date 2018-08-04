#include <unistd.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

const int item_size = 10;//队列大小
const int generate_size = 50;//要生产的个数


struct repo_t
{
    int items[item_size];
	std::mutex item_lock;
	int read_point;
	int write_point;
	std::condition_variable repo_not_null;//等待读取
	std::condition_variable repo_not_full;//等待写入
	
};
repo_t g_repo;


void init_repo(repo_t &repo)
{
	repo.read_point = 0;
	repo.write_point = 0;
	
}

void produce_item(repo_t &rep, int i)
{
	std::unique_lock<std::mutex> lock(rep.item_lock);
	while ((rep.write_point + 1) % item_size == rep.read_point)
	{
		cout << "wait for rep not full" << endl;
		rep.repo_not_full.wait_for(lock,std::chrono::seconds(1));
	}
	rep.items[rep.write_point++] = i;
	if (rep.write_point == item_size)
		rep.write_point = 0;
	rep.repo_not_null.notify_all();
	//lock.un_lock();
	
}
void producer_task()
{
	cout << "producer task statr" << endl;
	for (int i = 0; i < generate_size; i++)
	{
		cout<< "produce the " << i << " ^th item" << endl;
		produce_item(g_repo, i);
	}
	cout << "produce task finished!" << endl;
}

int consume_item(repo_t &rep)
{
	std::unique_lock<std::mutex> lock(rep.item_lock);
	while (rep.read_point == rep.write_point)
	{
		cout << "wait rep not null" << endl;
		rep.repo_not_null.wait_for(lock, std::chrono::seconds(1));
	}

	int data = rep.items[rep.read_point];
	rep.read_point++;
	if (rep.read_point == item_size)
		rep.read_point = 0;

	rep.repo_not_full.notify_all();
	return data;
	
}

void consumer_task()
{
	cout << "consumer task start" << endl;
	int item_count = 0;
	while (1)
	{
		sleep(3);
		int item = consume_item(g_repo);
		item_count ++;
		cout << "consume the " << item_count << "^th item, data:" << item << endl;
		if (item_count == generate_size)
			break;
	}
	cout << "consume task finished!" << endl;
}

int main(int argc, char **argv)
{
	init_repo(g_repo);
	std::thread producer(producer_task);
	std::thread consumer(consumer_task);
	producer.join();
	consumer.join();
	return 0;
}




