#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <functional>

namespace ReliefOps {
    namespace Logistics {

        // ====================================================================
        // UNIT IV: GENERIC TEMPLATE CLASS - PriorityDispatchQueue<T, Compare>
        // ====================================================================
        // A generic priority queue template container parameterized over payload
        // data type T and comparator functor Compare.
        template <typename T, typename Compare = std::less<T>>
        class PriorityDispatchQueue {
        private:
            std::vector<T> heapData; // Underlying STL container
            Compare comp;            // Functor object for element ordering

        public:
            // Default Constructor
            PriorityDispatchQueue() : heapData(), comp() {}

            // Parameterized Constructor accepting custom comparator
            explicit PriorityDispatchQueue(const Compare& comparator)
                : heapData(), comp(comparator) {}

            // Enqueue element and restore heap invariant
            void push(const T& item) {
                heapData.push_back(item);
                std::push_heap(heapData.begin(), heapData.end(), comp);
            }

            // Dequeue highest priority element
            void pop() {
                if (isEmpty()) {
                    throw std::underflow_error("PriorityDispatchQueue underflow: Cannot pop from empty queue.");
                }
                std::pop_heap(heapData.begin(), heapData.end(), comp);
                heapData.pop_back();
            }

            // Inspect highest priority element without removal
            const T& top() const {
                if (isEmpty()) {
                    throw std::underflow_error("PriorityDispatchQueue underflow: Queue is empty.");
                }
                return heapData.front();
            }

            // Query empty status
            bool isEmpty() const {
                return heapData.empty();
            }

            // Query element count
            size_t size() const {
                return heapData.size();
            }

            // Clear all queued items
            void clear() {
                heapData.clear();
            }

            // Access underlying elements for inspection (read-only)
            const std::vector<T>& getElements() const {
                return heapData;
            }
        };

    } // namespace Logistics
} // namespace ReliefOps

#endif // PRIORITY_QUEUE_HPP
