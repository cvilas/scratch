// single producer multi consumer queue
// chatGPT generated solution

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <print>
#include <thread>
#include <vector>

template <typename T>
class Queue {
public:
  struct ReaderContext {
    std::size_t init_index{};
    std::size_t num_reads{};
  };

  Queue(std::size_t size) : size_(size), buffer_(size_) {
  }

  auto newReaderContext() -> ReaderContext {
    return { .init_index = num_writes_ % size_, .num_reads = 0u };
  }

  void enqueue(const T& v) {  // Unsafe for multiple producers calling without external
                              // synchronisation
    const auto loc = num_writes_.load();
    buffer_.at(loc % size_) = v;
    num_writes_.store(loc + 1);
  }

  auto dequeue(ReaderContext& ctx) -> std::optional<T> {
    const auto num_writes = num_writes_.load(std::memory_order_relaxed);
    const auto num_reads_pending = num_writes - ctx.num_reads;

    // nothing to read
    if (num_reads_pending == 0) {
      return std::nullopt;
    }

    // writes overtook read. reset reads pointer
    if (num_reads_pending > size_) {
      ctx.num_reads = num_writes;
      return std::nullopt;
    }

    const auto loc = ctx.init_index + ctx.num_reads;
    T v = buffer_.at(loc % size_);
    ctx.num_reads++;
    return v;
  }

private:
  std::size_t size_{ 0 };
  std::vector<T> buffer_{};
  std::atomic<std::size_t> num_writes_{ 0 };  // TODO: make it atomic
};

// Example usage
int main() {
  constexpr size_t queue_size = 1000;
  Queue<int> queue(queue_size);

  auto read_ptr1 = queue.newReaderContext();
  auto read_ptr2 = queue.newReaderContext();

  // Producer
  std::thread producer([&queue]() {
    int i = 0;
    while (1) {
      queue.enqueue(++i);
      std::println("Produced: {}", i);
      std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  // Consumers

  std::thread consumer1([&queue, &read_ptr1 = read_ptr1]() {
    std::optional<int> item;
    while (1) {
      while (!(item = queue.dequeue(read_ptr1))) {
        std::this_thread::yield();
        // std::println("consumer 1 waiting");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      std::println("Consumer 1 consumed: {}", *item);
    }
  });

  std::thread consumer2([&queue, &read_ptr2 = read_ptr2]() {
    std::optional<int> item;
    while (1) {
      while (!(item = queue.dequeue(read_ptr2))) {
        std::this_thread::yield();
        // std::println("consumer 2 waiting");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      std::println("Consumer 2 consumed: {}", *item);
    }
  });

  producer.join();
  consumer1.join();
  consumer2.join();

  return 0;
}
