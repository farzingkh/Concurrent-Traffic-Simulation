#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
std::mutex MessageQueue<T>::_mutex;

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);
    _condition.wait(lck, [this] { return _dataAvailable; });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    _dataAvailable = false;
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&m)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    std::cout << "Message:  " << m << " added by " << std::this_thread::get_id() << std::endl;
    _queue.push_back(std::move(m));
    _dataAvailable = true;
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_mQueue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    while (true)
    {
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        srand((unsigned)time(0));
        int ranDur = 4000 + (rand() % 2001);
        if (dur >= ranDur)
        {
            start = std::chrono::system_clock::now();
            _currentPhase = static_cast<TrafficLightPhase>((_currentPhase + 1) % 2);
            _mQueue.send(std::move(_currentPhase));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
