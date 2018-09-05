#pragma once
#ifndef SINGLETON_HEADER_INCLUDED
#define SINGLETON_HEADER_INCLUDED

#include <utility>

template <class T>
class singleton
{
public:
  template <typename... Args>
  static
  T* get_instance(Args... args)
  {
    if (!instance_)
      {
        instance_ = new T(std::forward<Args>(args)...);
      }

    return instance_;
  }

  static
  void destroy_instance()
  {
    delete instance_;
    instance_ = nullptr;
  }

private:
  static T* instance_;
};

template <class T>
T*  singleton<T>::instance_ = nullptr;

#endif
