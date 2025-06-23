#pragma once

// Core Headers
#include "Logging.h"

// C++ Standard Library Headers
#include <iostream>

// External Vendor Library Headers

#ifdef NDEBUG

	#define BC_ASSERT(condition, message) ((void)0)

#else

    #define BC_ASSERT(condition, message)                                  \
        do {                                                                    \
            if (!(condition)) {                                                 \
                BC_CORE_FATAL("BC Assert: Failure Mesg - {0}", message);        \
                BC_CORE_FATAL("BC Assert: Failure File - {0}", __FILE__);       \
                BC_CORE_FATAL("BC Assert: Failure Line - {0}", __LINE__);       \
                std::terminate();                                               \
            }                                                                   \
        } while (false)
        
#endif

/// @brief Works like an Assert but rather than terminating the program, it will throw an exception to be caught
#define BC_THROW(condition, message)                                           \
    do {                                                                            \
        if (!(condition)) {                                                         \
            BC_CORE_FATAL("BC Throw: Runtime Error Encountered - {0}", message);    \
            throw std::runtime_error(message);                                      \
        }                                                                           \
    } while (false)

#define BC_CATCH_BEGIN() try { 
#define BC_CATCH_END() } catch (const std::exception& e) { BC_CORE_FATAL("BC Exception Caught: {}", e.what()); }
#define BC_CATCH_END_FUNC(func) } catch (const std::exception& e) { BC_CORE_FATAL("BC Exception Caught: {}", e.what()); func(); }
#define BC_CATCH_END_RETURN(return_value) } catch (const std::exception& e) { BC_CORE_FATAL("BC Exception Caught: {}", e.what()); return return_value; }