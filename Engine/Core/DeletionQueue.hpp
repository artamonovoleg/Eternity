#pragma once

#include <deque>
#include <set>
#include <functional>

class DeletionQueue
{
    private:
	    std::deque<std::function<void()>>   deletors;
    public:
        void PushDeleter(std::function<void()>&& function) 
        {
            deletors.push_back([=]()
            { 
                static bool firstcall = true;
                if (firstcall)
                {
                    function();
                    firstcall = false;
                }
            });
        }

        void Flush() 
        {
            // reverse iterate the deletion queue to execute all the functions
            for (auto it = deletors.rbegin(); it != deletors.rend(); it++) 
            {
                (*it)(); //call functors
            }

            deletors.clear();
        }
};