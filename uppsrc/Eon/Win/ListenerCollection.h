////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <mutex>
#include <vector>

// ListenerCollection
// Collection of raw pointers; owner is external and must unregister on teardown.
template<typename T>
class ListenerCollection
{
public:
    void Add(T* listener)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_listeners.push_back(listener);
    }

    void Remove(T* listener)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        erase_if(&m_listeners, [listener](T* ptr) { return ptr == listener; });
    }

    std::vector<T*> GetListeners() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_listeners;
    }

private:
    mutable std::mutex m_mutex;
    std::vector<T*> m_listeners;
};
