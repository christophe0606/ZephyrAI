/*
 * Zephyr-compatible type definitions for ExecuTorch cross-compilation.
 * 
 * This header forces int32_t to use 'int' instead of 'long int' to match
 * Zephyr's type definitions (see zephyr/toolchain/zephyr_stdint.h).
 * 
 * Without this, the ExecuTorch libraries get different C++ name mangling
 * than the Zephyr application, causing undefined reference errors.
 */

#ifndef ZEPHYR_STDINT_COMPAT_H_
#define ZEPHYR_STDINT_COMPAT_H_

#undef __INT32_TYPE__
#undef __UINT32_TYPE__
#undef __INT_FAST32_TYPE__
#undef __UINT_FAST32_TYPE__
#undef __INT_LEAST32_TYPE__
#undef __UINT_LEAST32_TYPE__
#undef __INT64_TYPE__
#undef __UINT64_TYPE__
#undef __INT_FAST64_TYPE__
#undef __UINT_FAST64_TYPE__
#undef __INT_LEAST64_TYPE__
#undef __UINT_LEAST64_TYPE__

#define __INT32_TYPE__ int
#define __UINT32_TYPE__ unsigned int
#define __INT_FAST32_TYPE__ __INT32_TYPE__
#define __UINT_FAST32_TYPE__ __UINT32_TYPE__
#define __INT_LEAST32_TYPE__ __INT32_TYPE__
#define __UINT_LEAST32_TYPE__ __UINT32_TYPE__
#define __INT64_TYPE__ long long int
#define __UINT64_TYPE__ unsigned long long int
#define __INT_FAST64_TYPE__ __INT64_TYPE__
#define __UINT_FAST64_TYPE__ __UINT64_TYPE__
#define __INT_LEAST64_TYPE__ __INT64_TYPE__
#define __UINT_LEAST64_TYPE__ __UINT64_TYPE__

#endif /* ZEPHYR_STDINT_COMPAT_H_ */
