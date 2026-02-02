/**
 * @file report.h
 *
 * Provides visualization tools for analysing cache side-channel data.
 */

#ifndef AXIUM_VISUALIZATION_REPORT_H
#define AXIUM_VISUALIZATION_REPORT_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Results and analysis structure.
 */
typedef struct {
  uint64_t threshold;           /**< Original calibrated threshold. */
  uint64_t effective_threshold; /**< Refined threshold after analysis. */
  uint64_t *timings;            /**< Raw timing results. */
  size_t count;                 /**< Number of elements. */
  int winner_idx;               /**< Index of the global minimum. */
  uint64_t winner_val;          /**< Timing of the winner. */
  uint64_t gap;                 /**< Timing gap between winner and runner-up. */
  size_t hits_count;            /**< Refined count of valid hits. */
} cache_report_t;

/**
 * @brief Report structure for cache watching.
 */
typedef struct {
  uint64_t *hit_counts; /**< Array of hit counts per index. */
  size_t count;         /**< Number of monitored lines. */
  uint64_t threshold;   /**< Cycle threshold used. */
  uint64_t total_hits;  /**< Total number of hits detected. */
} cache_watch_report_t;

/**
 * @brief Initializes a cache watch report structure.
 *
 * @param report Pointer to the report structure to initialize.
 * @param hit_counts Array to store hit counts (must be at least 'count'
 * elements).
 * @param count Number of monitored lines.
 * @param threshold Cycle threshold used for detection.
 */
void cache_watch_report_init(cache_watch_report_t *report, uint64_t *hit_counts,
                             size_t count, uint64_t threshold);

/**
 * @brief Prints a visual summary of the side-channel results.
 *
 * @param report Pointer to the analysis results to display.
 */
void cache_report(const cache_report_t *report);

/**
 * @brief A default reporter callback that populates a cache_watch_report_t.
 *
 * @param index The index of the memory line that was hit.
 * @param cycles The measured access time.
 * @param user_data Must be a pointer to a cache_watch_report_t.
 */
void cache_watch_reporter(size_t index, uint64_t cycles, void *user_data);

/**
 * @brief Analyzes timing data using Gap-based noise reduction.
 *
 * @param report Pointer to the report structure to populate.
 * @param timings Array of raw timing measurements.
 * @param count Number of elements in the timings array.
 * @param threshold Initial cycle threshold for hit detection.
 */
void cache_analyze(cache_report_t *report, uint64_t *timings, size_t count,
                   uint64_t threshold);

/**
 * @brief Exports cache analysis data to a JSON file for visualization.
 *
 * @param report Pointer to the analysis report.
 * @param filename Path to the output JSON file.
 * @return 0 on success, -1 on failure.
 */
int cache_export_report(const cache_report_t *report, const char *filename);

/**
 * @brief Exports a watch report to a JSON file.
 *
 * @param report The report to export.
 * @param filename Output filename.
 * @return 0 on success, -1 on failure.
 */
int cache_export_watch_report(const cache_watch_report_t *report,
                              const char *filename);

/**
 * @brief Opens the visualization tool to view a specific report.
 *
 * This function attempts to open the `tools/cache_vis.html` file in the
 * default web browser, passing the report path as a parameter.
 *
 * @param filename Path to the exported JSON report.
 */
void cache_view_report(const char *filename);

#endif /* AXIUM_VISUALIZATION_REPORT_H */
