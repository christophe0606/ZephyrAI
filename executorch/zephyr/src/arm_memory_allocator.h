/* Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 * Copyright 2025 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <executorch/runtime/core/memory_allocator.h>


/**
 * A simple memory allocator for ARM embedded targets that manages a static
 * memory pool. This allocator is designed for bare-metal and RTOS environments
 * where dynamic memory allocation should be avoided.
 *
 * The allocator maintains a simple bump-pointer allocation scheme within
 * the provided buffer.
 */
class ArmMemoryAllocator : public executorch::runtime::MemoryAllocator {
 public:
  /**
   * Construct an ArmMemoryAllocator with the given buffer.
   *
   * @param size The size of the memory buffer in bytes.
   * @param buffer Pointer to the memory buffer to use for allocations.
   */
  ArmMemoryAllocator(uint32_t size, uint8_t* buffer);

  /**
   * Allocate memory from the pool.
   *
   * @param size The number of bytes to allocate.
   * @param alignment The alignment requirement for the allocation.
   * @return A pointer to the allocated memory, or nullptr if allocation fails.
   */
  void* allocate(size_t size, size_t alignment = kDefaultAlignment) override;

  /**
   * Reset the allocator, freeing all allocations.
   * After reset, the entire buffer is available for new allocations.
   */
  void reset() override;

  /**
   * Get the number of bytes currently used.
   * @return The number of bytes allocated from the pool.
   */
  size_t used_size() const {
    return offset_;
  }

  /**
   * Get the number of bytes still available.
   * @return The number of bytes remaining in the pool.
   */
  size_t free_size() const {
    return size_ - offset_;
  }

 private:
  uint8_t* buffer_;
  uint32_t size_;
  uint32_t offset_;
};

