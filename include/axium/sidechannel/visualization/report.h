/**
 * @file report.h
 *
 * Provides visualization tools for analysing cache side-channel data.
 */

#ifndef AXIUM_VISUALIZATION_REPORT_H
#define AXIUM_VISUALIZATION_REPORT_H

#include <axium/sidechannel/cache.h>

/**
 * @brief Prints a visual summary of the side-channel results.
 *
 * @param report Pointer to the analysis results to display.
 */
void cache_report(const cache_report_t *report);

/**
 * @brief Exports cache analysis data to a JSON file for visualization.
 *
 * @param report Pointer to the analysis report.
 * @param filename Path to the output JSON file.
 * @return 0 on success, -1 on failure.
 */
int cache_export_report(const cache_report_t *report, const char *filename);

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
