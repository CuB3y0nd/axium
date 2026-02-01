#include <axium/axium.h>

void simulate_scenario(const char *name, size_t count, int target_idx,
                       int noise_level, int scenario_type) {
  uint64_t *timings = malloc(sizeof(uint64_t) * count);

  // Base miss latency with noise
  for (size_t i = 0; i < count; i++) {
    timings[i] = 220 + (rand() % noise_level);
  }

  if (scenario_type == 0) { // Ideal
    timings[target_idx] = 40 + (rand() % 10);
  } else if (scenario_type == 1) { // Noisy
    timings[target_idx] = 70 + (rand() % 20);
    // Add occasional random noise floor drops (distractors)
    for (int j = 0; j < 5; j++)
      timings[rand() % count] = 120 + (rand() % 30);
  } else if (scenario_type == 2) { // Prefetcher (Multiple adjacent hits)
    for (int j = -2; j <= 2; j++) {
      int idx = target_idx + j;
      if (idx >= 0 && idx < (int)count) {
        timings[idx] = 50 + (abs(j) * 25) + (rand() % 10);
      }
    }
  }

  cache_report_t report;
  cache_analyze(&report, timings, count, 150);

  char filename[64];
  snprintf(filename, sizeof(filename), "report_%s.json", name);
  cache_export_report(&report, filename);

  log_success("Generated %s scenario to %s", name, filename);
  free(timings);
}

void simulate_watch_scenario(const char *name, size_t count, int target_idx) {
  uint64_t *hits = calloc(count, sizeof(uint64_t));
  cache_watch_report_t report;

  cache_watch_report_init(&report, hits, count, 150);

  // Simulate hits with a more complex distribution
  for (int i = 0; i < 1000; i++) {
    int idx;
    int roll = rand() % 100;

    if (roll < 35) {
      // 35% Primary target
      idx = target_idx;
    } else if (roll < 55) {
      // 20% Spatial locality / Prefetcher (neighboring lines)
      int offset = (rand() % 3) - 1; // -1, 0, 1
      idx = (target_idx + offset + (int)count) % (int)count;
    } else if (roll < 70) {
      // 15% Distractor (another frequent access point)
      idx = (target_idx + (int)count / 2) % (int)count;
    } else {
      // 30% Uniform background noise
      idx = rand() % count;
    }

    cache_watch_reporter(idx, 40 + (rand() % 40), &report);
  }

  char filename[64];
  snprintf(filename, sizeof(filename), "report_%s.json", name);
  cache_export_watch_report(&report, filename);

  log_success("Generated watch scenario to %s", filename);
  free(hits);
}

int main() {
  srand(time(NULL));
  log_info("Generating advanced simulation datasets...");

  simulate_scenario("ideal", 128, 42, 40, 0);
  simulate_scenario("noisy", 256, 100, 150, 1);
  simulate_scenario("prefetcher", 256, 180, 50, 2);

  simulate_watch_scenario("watch_monitor", 64, 32);

  log_status(
      "All simulations complete. Open tools/cache_vis.html to view reports.");
  return 0;
}
