/*
 * CConcurrentQueue.h
 *
 *  Created on: 2017年10月19日
 *      Author: jugo
 */

#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>

#include <queue>

template<typename Data>
class CConcurrentQueue
{
public:
	CConcurrentQueue();
	virtual ~CConcurrentQueue();

	// push() 在末尾加入一个元素
	void push(Data const& data);

	// empty() 如果队列空则返回真
	bool empty() const;

	// pop() 删除第一个元素
	void pop(Data& popped_value);
	int size();

private:
	std::queue<Data> the_queue;
	mutable boost::mutex the_mutex;
};
