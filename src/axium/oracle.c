/**
 * @file oracle.c
 * @brief Implementation of generic oracle utilities with SCA optimizations.
 */

#include <axium/log.h>
#include <axium/oracle.h>
#include <axium/sidechannel/cpu.h>
#include <string.h>

/**
 * @brief Evaluates the statistical confidence of the current vote distribution.
 *
 * Uses a combination of Confidence Gap (Winner vs Runner-up) and
 * Signal-to-Noise Ratio (Winner vs Noise Floor).
 */
static int eval_confidence(const int *v, size_t n, int threshold,
                           int *out_winner) {
  int winner = -1;
  int max_v = 0;
  int second_v = 0;
  long total = 0;
  int num_max = 0;

  for (size_t i = 0; i < n; i++) {
    total += v[i];
    if (v[i] > max_v) {
      second_v = max_v;
      max_v = v[i];
      winner = (int)i;
      num_max = 1;
    } else if (v[i] == max_v && max_v > 0) {
      num_max++;
    } else if (v[i] > second_v) {
      second_v = v[i];
    }
  }

  if (max_v < threshold || num_max > 1) {
    return 0;
  }

  /* 1. Confidence Gap: Winner must have a significant lead (e.g., >12.5%) */
  int gap = max_v - second_v;
  int min_gap = (max_v >> 3) + 2;

  /* 2. Noise Floor: Winner must be significantly above the average votes */
  int avg = (int)(total / (long)n);
  if (gap < min_gap || max_v < (avg * 2)) {
    return 0;
  }

  if (out_winner)
    *out_winner = winner;
  return 1;
}

ssize_t oracle_scan(oracle_t *o, char *buf, size_t len, int terminator) {
  if (!o || !buf)
    return -1;

  size_t i;
  for (i = 0; i < len; i++) {
    int res = oracle_query(o, i);
    if (res == -1)
      break;

    buf[i] = (char)res;
    if (terminator >= 0 && res == terminator) {
      i++;
      break;
    }
  }

  return (ssize_t)i;
}

int oracle_query_stat(oracle_t *o, size_t input, int rounds, int threshold,
                      int *votes, size_t n_candidates) {
  if (!o || (rounds <= 0 && !votes))
    return -1;

  int *v = votes;
  size_t n = n_candidates;

  for (int r = 0; r < rounds; r++) {
    int res = oracle_query(o, input);
    if (res >= 0 && (size_t)res < n) {
      v[res]++;
    }

    /* Early exit check every 25% of rounds to improve performance */
    if (r > 0 && (r % (rounds / 4 + 1) == 0)) {
      int winner;
      if (eval_confidence(v, n, threshold, &winner)) {
        return winner;
      }
    }
  }

  int winner;
  return eval_confidence(v, n, threshold, &winner) ? winner : -1;
}

ssize_t oracle_scan_stat(oracle_t *o, char *buf, size_t len, int terminator,
                         int rounds, int threshold, int max_retries, int *votes,
                         size_t n_candidates) {
  if (!o || !buf)
    return -1;

  int local_votes[256];
  int *v = votes ? votes : local_votes;
  size_t n = n_candidates ? n_candidates : 256;

  size_t i;
  for (i = 0; i < len; i++) {
    int res = -1;
    int attempt = 0;

    memset(v, 0, n * sizeof(int));

    while (res == -1 && attempt < max_retries) {
      res = oracle_query_stat(o, i, rounds, threshold, v, n);
      if (res == -1) {
        attempt++;
        if (attempt < max_retries) {
          /* Log weak signal periodically */
          if (attempt % 5 == 0) {
            log_status(
                "Weak signal at index %zu (attempt %d, total samples: %d)", i,
                attempt, attempt * rounds);
          }

          /* Adaptive Backoff Logic:
           * - Early retries: yield CPU to handle system interrupts.
           * - Deep retries: busy-wait with exponential delay_cycles to
           *   allow cache/bus transients to settle.
           */
          if (attempt < 5) {
            sched_yield();
          } else {
            delay_cycles(10000UL << (attempt > 10 ? 10 : attempt));
          }

          /* Memory Decay: If stuck, attenuate old samples instead of clearing.
           * This filters out transient noise while keeping the signal trend.
           */
          if (attempt % (max_retries / 3 + 1) == 0) {
            log_status("High noise at index %zu, decaying memory floor...", i);
            for (size_t j = 0; j < n; j++)
              v[j] >>= 1;
          }
        }
      }
    }

    if (res == -1) {
      log_error("Lost signal at index %zu after %d samples.", i,
                attempt * rounds);
      break;
    }

    buf[i] = (char)res;
    log_info("Leaked [%3zu]: '%c' (0x%02x) | Attempts: %-3d", i,
             (res >= 32 && res < 127) ? res : '?', res, attempt + 1);

    if (terminator >= 0 && res == terminator) {
      i++;
      break;
    }
  }

  return (ssize_t)i;
}
