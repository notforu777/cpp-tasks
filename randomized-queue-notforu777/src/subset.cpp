#include "subset.h"

#include "randomized_queue.h"

void subset(unsigned long k, std::istream & in, std::ostream & out)
{
    std::string line;
    randomized_queue<std::string> queue;
    unsigned long k_copy = k;
    while (k > 0 && std::getline(in, line)) {
        queue.enqueue(line);
        --k;
    }
    try {
        for (unsigned long i = 0; i < k_copy; ++i) {
            out << queue.dequeue() << '\n';
        }
    }
    catch (std::ios_base::failure & e) {
    }
}
