#ifndef MPSCQ_H
#define MPSCQ_H

#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <optional>
#include <vector>

/// multi-producer, single consumer queue.
/// Inspired from https://github.com/dbittman/waitfree-mpsc-queue
template <typename T, size_t capacity_>
class mpscq
{
public:
  mpscq();
  bool tryPush(T &&obj);
  std::optional<T> tryPop();
  size_t count();

private:
  std::atomic<size_t> count_{0};
  std::atomic<size_t> head_{0};
  size_t tail_{0};
  std::vector<std::atomic<bool>> is_readable_;
  std::vector<T> buffer_;
};

template <typename T, size_t capacity_>
mpscq<T, capacity_>::mpscq() : is_readable_(capacity_), buffer_(capacity_)
{
  assert(capacity_ >= 1);
  for (auto &i : is_readable_) {
    i = false;
  }
}

/// Attempt to enqueue without blocking. This is safe to call from multiple threads.
/// \return true on success and false if queue is full.
template <typename T, size_t capacity_>
bool mpscq<T, capacity_>::tryPush(T &&obj)
{
  auto count = count_.fetch_add(1, std::memory_order_acquire);
  if (count >= capacity_) {
    // back off, queue is full
    count_.fetch_sub(1, std::memory_order_release);
    return false;
  }

  // increment the head, which gives us 'exclusive' access to that element until
  // is_reabable_ flag is set
  const auto head = head_.fetch_add(1, std::memory_order_acquire) % capacity_;
  buffer_[head] = std::move(obj);
  assert(is_readable_[head] == false);
  is_readable_[head].store(true, std::memory_order_release);
  return true;
}

/// Attempt to dequeue without blocking
/// \note: This is not safe to call from multiple threads.
/// \return A valid item from queue if the operation won't block, else nothing
template <typename T, size_t capacity_>
std::optional<T> mpscq<T, capacity_>::tryPop()
{
  if (!is_readable_[tail_].load(std::memory_order_acquire)) {
    // A thread could still be writing to this location
    return {};
  }

  assert(is_readable_[tail_]);
  auto ret = std::move(buffer_[tail_]);
  is_readable_[tail_].store(false, std::memory_order_release);

  if (++tail_ >= capacity_) {
    tail_ = 0;
  }

  const auto count = count_.fetch_sub(1, std::memory_order_release);
  assert(count > 0);
  return ret;
}

/// \return The number of items in queue
template <typename T, size_t capacity_>
size_t mpscq<T, capacity_>::count()
{
  return count_.load(std::memory_order_relaxed);
}

#endif // MPSCQ_H
