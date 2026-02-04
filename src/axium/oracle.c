/**
 * @file oracle.c
 * @brief Implementation of generic oracle utilities.
 */

#include <axium/log.h>
#include <axium/oracle.h>

ssize_t oracle_scan(oracle_t *o, char *buf, size_t len, int terminator) {
  if (!o || !buf) {
    return -1;
  }

  size_t i;
  for (i = 0; i < len; i++) {
    int res = oracle_query(o, i);
    if (res == -1) {
      break;
    }

    buf[i] = (char)res;

    if (terminator >= 0 && res == terminator) {
      i++; /* Include the terminator in the count */
      break;
    }
  }

  return (ssize_t)i;
}

int oracle_query_stat(oracle_t *o, size_t input, int rounds, int threshold,
                      int *votes, size_t n_candidates) {
  if (!o || (rounds <= 0 && !votes))
    return -1;

  int local_votes[256] = {0};
  int *v = votes;
  size_t n = n_candidates;

  if (!v) {
    v = local_votes;
    n = 256;
  }

  int winner = -1;
  int max_votes = 0;
  int second_max_votes = 0;
  int num_max = 0;

  /* Identify current state if we are accumulating */
  for (size_t i = 0; i < n; i++) {
    if (v[i] > max_votes) {
      second_max_votes = max_votes;
      max_votes = v[i];
      winner = (int)i;
      num_max = 1;
    } else if (v[i] == max_votes && max_votes > 0) {
      num_max++;
    } else if (v[i] > second_max_votes) {
      second_max_votes = v[i];
    }
  }

  /* Perform new rounds */
  for (int r = 0; r < rounds; r++) {
    int res = oracle_query(o, input);
    if (res >= 0 && (size_t)res < n) {
      v[res]++;

      if (v[res] > max_votes) {
        second_max_votes = max_votes;
        max_votes = v[res];
        winner = res;
        num_max = 1;
      } else if (v[res] == max_votes) {
        num_max++;
      } else if (v[res] > second_max_votes) {
        second_max_votes = v[res];
      }
    }

    /* Mathematical early exit:
     * If the current leader cannot be overtaken by any other candidate
     * even if all remaining samples in THIS call go to that candidate.
     */
    int remaining = rounds - 1 - r;
    if (max_votes >= threshold && max_votes > (second_max_votes + remaining)) {
      if (num_max == 1)
        break;
    }
  }

  return (max_votes >= threshold && num_max == 1) ? winner : -1;
}

ssize_t oracle_scan_stat(oracle_t *o, char *buf, size_t len, int terminator,
                         int rounds, int threshold, int max_retries, int *votes,
                         size_t n_candidates) {
  if (!o || !buf)
    return -1;

  int local_votes[256];
  int *v = votes;
  size_t n = n_candidates;

  if (!v) {
    v = local_votes;
    n = 256;
  }

  size_t i;
  for (i = 0; i < len; i++) {
    int res = -1;
    int attempt = 0;

    /* Zero the votes buffer for each new byte */
    for (size_t j = 0; j < n; j++)
      v[j] = 0;

    /* Incremental retry: keep previous samples to increase SNR */
    while (res == -1 && attempt < max_retries) {
      res = oracle_query_stat(o, i, rounds, threshold, v, n);
      if (res == -1) {
        attempt++;
        if (attempt < max_retries) {
          log_status(
              "Weak signal at index %zu (total samples: %d), retrying...", i,
              attempt * rounds);
        }
      }
    }

    if (res == -1) {
      log_error("Lost signal at index %zu after %d samples.", i,
                max_retries * rounds);
      break;
    }

    buf[i] = (char)res;
    log_info("Leaked byte %zu: '%c' (statistically confirmed)", i,
             (res >= 32 && res < 127) ? res : '?');

    if (terminator >= 0 && res == terminator) {
      i++;
      break;
    }
  }

  return (ssize_t)i;
}
