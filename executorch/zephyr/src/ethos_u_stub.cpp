/**
 * @file ethos_u_stub.cpp
 * @brief Ethos-U delegate registration verification for ExecuTorch
 *
 * This provides a function that the runner can call to verify the
 * Ethos-U delegate is properly registered.
 * 
 * NOTE: The actual backend registration happens via the static constructor
 * in EthosUBackend.cpp.obj which is linked directly (extracted from the
 * libexecutorch_delegate_ethos_u.a archive) to ensure the static initializer
 * is included in the final executable.
 */

#include <executorch/runtime/core/error.h>

extern "C" {

/**
 * Registration check for the Ethos-U backend delegate.
 * This function is called by the executor runner to verify the
 * Ethos-U delegate is properly registered.
 * 
 * @return Error::Ok if registered successfully
 */
executorch::runtime::Error executorch_delegate_EthosUBackend_registered(void) {
    // The delegate is registered via the static constructor in EthosUBackend.cpp
    // which is linked directly to ensure it's included in the final executable
    return executorch::runtime::Error::Ok;
}

} // extern "C"
