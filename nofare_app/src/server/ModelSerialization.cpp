
#include "ModelSerialization.hpp"

namespace detail {
    std::atomic_bool shutdown;
    std::thread model_thread;

    std::mutex condition_mutex;
    std::condition_variable wakeup_var;
}