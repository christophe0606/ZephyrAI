/* Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 * Copyright 2025 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "arm_memory_allocator.h"

#include <cstdint>
#include <cstring>

ArmMemoryAllocator::ArmMemoryAllocator(uint32_t size, uint8_t* buffer)
    : MemoryAllocator(size, buffer), buffer_(buffer), size_(size), offset_(0) {}

void* ArmMemoryAllocator::allocate(size_t size, size_t alignment) {
  // Align the current offset
  size_t aligned_offset = (offset_ + alignment - 1) & ~(alignment - 1);

  // Check if we have enough space
  if (aligned_offset + size > size_) {
    return nullptr;
  }

  // Get the pointer to return
  void* ptr = buffer_ + aligned_offset;

  // Update the offset
  offset_ = aligned_offset + size;

  return ptr;
}

void ArmMemoryAllocator::reset() {
  offset_ = 0;
}

