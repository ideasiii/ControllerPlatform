/*
 * CConcurrentQueue.cpp
 *
 *  Created on: 2017年10月19日
 *      Author: jugo
 */

#include "CConcurrentQueue.h"

template<typename Data>
CConcurrentQueue<Data>::CConcurrentQueue()
{

}

template<typename Data>
CConcurrentQueue<Data>::~CConcurrentQueue()
{

}

template<typename Data>
void CConcurrentQueue<Data>::push(Data const& data)
{
	boost::mutex::scoped_lock lock(the_mutex);
	the_queue.push(data);
}

template<typename Data>
bool CConcurrentQueue<Data>::empty() const
{
	boost::mutex::scoped_lock lock(the_mutex);
	return the_queue.empty();
}

template<typename Data>
void CConcurrentQueue<Data>::pop(Data& popped_value)
{
	boost::mutex::scoped_lock lock(the_mutex);
	popped_value = the_queue.front();
	the_queue.pop();
}

template<typename Data>
int CConcurrentQueue<Data>::size()
{
	boost::mutex::scoped_lock lock(the_mutex);
	return the_queue.size();
}

