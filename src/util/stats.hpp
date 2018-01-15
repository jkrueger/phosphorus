#pragma once

#include <atomic>
#include <memory>

struct stats_t {
  typedef std::shared_ptr<stats_t> p;
  
  std::atomic<uint32_t> areas;
  std::atomic<uint32_t> rays;

  stats_t()
    : areas(0), rays(0)
  {}
};
