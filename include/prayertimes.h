/* libprayertimes public API. */
#ifndef PRAYER_TIMES_H_
#define PRAYER_TIMES_H_

#include <stdbool.h>
#include <time.h>

typedef struct prayertimes_conf {
    /* --- time calculation --- */

    /* Length of a shadow at which Asr takes place. */
    double asr_shadow_length;

    /* Angles relative to [solar] noon at which Fajr, Maghrib,
     * and Isha take place. */
    double fajr_angle, maghrib_angle, isha_angle;

    /* How many minutes to delay Maghrib and Isha. */
    double maghrib_minutes, isha_minutes;

    /* Use sunset for Maghrib. */
    bool maghrib_use_sunset;

    /* Use Maghrib for Isha. Supposed to be used in combination
     * with isha_minutes. */
    bool isha_use_maghrib;

    /* --- calculation information --- */

    /* Latitude of calculation. North is positive, south is negative */
    double latitude;

    /* Longitude of calculation. East is positive, west is negative */
    double longitude;

    /* Elevation, relative to sea level, in meters for the calculation.
     * This value is optional and if you don't know/don't care, set it
     * to 0. */
    double elevation;

    /* --- timezone tomfoolery --- */

    /* Set to true to force using the timezone `timezone`.
     * If false, assume system timezone. */
    bool force_timezone;

    /* Timezone that is used if `force_timezone` is true.
     * Expressed in minutes rather than hours. 0 is UTC. */
    double timezone;
} prayertimes_conf_t;

/* Calculate prayer times into `times` given configuration `conf` and
 * time `time`. */
void prayertimes_calc(prayertimes_conf_t conf, time_t time, double times[7]);
/* Adjust prayer times into `out_times` given configuration `conf`,
 * time `time` and unadjusted times `in_times`. */
void prayertimes_adjust(prayertimes_conf_t conf, time_t time,
                        double in_times[7], double out_times[7]);

/* Return version of libprayertimes */
const char *prayertimes_version(void);

#endif /* PRAYER_TIMES_H_ */
