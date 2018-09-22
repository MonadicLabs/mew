#pragma once
#include <atomic>

  // A lock-free stack.
  // Push = single producer
  // Pop = single consumer (same thread as push)
  // Steal = multiple consumer

  // All methods, including Push, may fail. Re-issue the request
  // if that occurs (spinwait).

  template<class T, size_t capacity = 131072>
  class WorkStealingStack {

  public:
    inline WorkStealingStack() {
      _top = 1;
      _bottom = 1;
    }

    WorkStealingStack(const WorkStealingStack&) = delete;

    inline ~WorkStealingStack()
    {

    }

    inline size_t size()
    {
        auto oldtop = _top.load(std::memory_order_relaxed);
        auto oldbottom = _bottom.load(std::memory_order_relaxed);
        auto numtasks = oldbottom - oldtop;
        return numtasks;
    }

    // Single producer
    inline bool Push(const T& item) {
      auto oldtop = _top.load(std::memory_order_relaxed);
      auto oldbottom = _bottom.load(std::memory_order_relaxed);
      auto numtasks = oldbottom - oldtop;

//      cerr << "queue_size=" << numtasks << endl;

      if (
        oldbottom > oldtop && // size_t is unsigned, validate the result is positive
        numtasks >= capacity - 1) {
        // The caller can decide what to do, they will probably spinwait.
        return false;
      }

      _values[oldbottom % capacity].store(item, std::memory_order_relaxed);
      _bottom.fetch_add(1, std::memory_order_release);
      return true;
    }

    // Single consumer
    inline bool Pop(T& result) {

      size_t oldtop, oldbottom, newtop, newbottom, ot;

      oldbottom = _bottom.fetch_sub(1, std::memory_order_release);
      ot = oldtop = _top.load(std::memory_order_acquire);
      newtop = oldtop + 1;
      newbottom = oldbottom - 1;

      // Bottom has wrapped around.
      if (oldbottom < oldtop) {
        _bottom.store(oldtop, std::memory_order_relaxed);
        return false;
      }

      // The queue is empty.
      if (oldbottom == oldtop) {
        _bottom.fetch_add(1, std::memory_order_release);
        return false;
      }

      // Make sure that we are not contending for the item.
      if (newbottom == oldtop) {
        auto ret = _values[newbottom % capacity].load(std::memory_order_relaxed);
        if (!_top.compare_exchange_strong(oldtop, newtop, std::memory_order_acquire)) {
          _bottom.fetch_add(1, std::memory_order_release);
          return false;
        }
        else {
          result = ret;
          _bottom.store(newtop, std::memory_order_release);
          return true;
        }
      }

      // It's uncontended.
      result = _values[newbottom % capacity].load(std::memory_order_acquire);
      return true;
    }

    // Multiple consumer.
    inline bool Steal(T& result) {
      size_t oldtop, newtop, oldbottom;

      oldtop = _top.load(std::memory_order_acquire);
      oldbottom = _bottom.load(std::memory_order_relaxed);
      newtop = oldtop + 1;

      if (oldbottom <= oldtop)
        return false;

      // Make sure that we are not contending for the item.
      if (!_top.compare_exchange_strong(oldtop, newtop, std::memory_order_acquire)) {
        return false;
      }

      result = _values[oldtop % capacity].load(std::memory_order_relaxed);
      return true;

    }

  private:

    // Circular array
    std::atomic<T> _values[capacity];
    std::atomic<size_t> _top; // queue
    std::atomic<size_t> _bottom; // stack
  };
