/* 2015 Daniel Bittman <danielbittman1@gmail.com>: http://dbittman.github.io/ */
#ifndef __MPSCQ_H
#define __MPSCQ_H

#include <cinttypes>
#include <atomic>
#include <cstddef>
#include <vector>
#include <cassert>

/// multi-producer, single consumer queue.
template<typename T, size_t N>
class mpscq
{
public:
  mpscq();
  bool push(T&& obj);
  std::tuple<bool, T> pop();
  size_t count();

private:
  std::atomic<size_t> count_{0};
  std::atomic<size_t> head_{0};
  size_t tail_{0};
  std::vector<T> buffer_;
  std::vector<std::atomic<bool>> is_readable_;
};

template<typename T, size_t N>
mpscq<T,N>::mpscq() : buffer_(N), is_readable_(N)
{
  assert(N >= 1);
  for(auto& i : is_readable_)
  {
    i = false;
  }
}

/// Enqueue an item into the queue. This is safe to call from multiple threads.
/// \return true on success and false on failure (queue full).
template<typename T, size_t N>
bool mpscq<T,N>::push(T&& obj)
{
  auto count = count_.fetch_add(1, std::memory_order_acquire);
  if(count >= N)
  {
    // back off, queue is full
    count_.fetch_sub(1, std::memory_order_release);
    return false;
  }

  // increment the head, which gives us 'exclusive' access to that element until is_reabable_ flag is set
  auto head = head_.fetch_add(1, std::memory_order_acquire);
  buffer_[head % N] = std::move(obj);
  is_readable_[head % N] = true;
  return true;

}

/// Dequeue an item from the queue and return it.
/// \note: This is not safe to call from multiple threads.
/// \return {true, object} on success, and the item it dequeued on success
template<typename T, size_t N>
std::tuple<bool, T> mpscq<T,N>::pop()
{
  if(!is_readable_[tail_])
  {
    // A thread could still be writing to this location
    return {false, std::move(T())};
  }

  auto ret = std::move(buffer_[tail_]);
  is_readable_[tail_] = false;

  if(++tail_ >= N)
  {
    tail_ = 0;
  }

  size_t r = count_.fetch_sub(1, std::memory_order_release);
  assert(r > 0);
  return {true, std::move(ret)};
}

/// \return The number of items in the queue currently
template<typename T, size_t N>
size_t mpscq<T,N>::count()
{
  return count_.load(std::memory_order_relaxed);
}

#endif

