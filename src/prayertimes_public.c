/* libprayertimes public API. */
#include "prayertimes.h"
#include "prayertimesC.h"

/* Calculate prayer times into `times` given configuration `conf` and
 * time `time`. */
void prayertimes_calc(prayertimes_conf_t conf, time_t time, double times[7])
{
    times_conf c;
    c = (times_conf){ .asr_shadow_length = conf.asr_shadow_length,
                      .fajr_angle = conf.fajr_angle,
                      .maghrib_angle = conf.maghrib_angle,
                      .isha_angle = conf.isha_angle,
                      .maghrib_minutes = conf.maghrib_minutes,
                      .isha_minutes = conf.isha_minutes,
                      .use_isha_angle = !conf.isha_use_maghrib,
                      .use_maghrib_angle = !conf.maghrib_use_sunset,
                      .adjust_method = 0 /* unused */ };
    if(!times) {
        return;
    }

    double tz = 0;
    if(conf.force_timezone) {
        tz = conf.timezone;
    } else {
        time_t do_not_modify = time;
        struct tm tm = *localtime(&do_not_modify);
        struct tm utc_tm = *gmtime(&do_not_modify);
        /* Thanks https://stackoverflow.com/questions/13804095/get-the-time-zone-gmt-offset-in-c! */
        utc_tm.tm_isdst = -1;
        long off = mktime(&tm) - mktime(&utc_tm);
        tz = (double)off / 3600.0;
    }

    calc_schedule(conf.latitude, conf.longitude, conf.elevation, tz, time,
                  times, c);

    return;
}

/* Adjust prayer times into `out_times` given configuration `conf`,
 * time `time` and unadjusted times `in_times`. */
void prayertimes_adjust(prayertimes_conf_t conf, time_t time,
                        double in_times[7], double out_times[7])
{
    times_conf c;
    c = (times_conf){ .asr_shadow_length = conf.asr_shadow_length,
                      .fajr_angle = conf.fajr_angle,
                      .maghrib_angle = conf.maghrib_angle,
                      .isha_angle = conf.isha_angle,
                      .maghrib_minutes = conf.maghrib_minutes,
                      .isha_minutes = conf.isha_minutes,
                      .use_isha_angle = !conf.isha_use_maghrib,
                      .use_maghrib_angle = !conf.maghrib_use_sunset,
                      .adjust_method = 0 /* unused */ };
    if(!in_times || !out_times) {
        return;
    }

    double tz = 0;
    if(conf.force_timezone) {
        tz = conf.timezone;
    } else {
        time_t do_not_modify = time;
        struct tm tm = *localtime(&do_not_modify);
        struct tm utc_tm = *gmtime(&do_not_modify);
        /* Thanks https://stackoverflow.com/questions/13804095/get-the-time-zone-gmt-offset-in-c! */
        utc_tm.tm_isdst = -1;
        long off = mktime(&tm) - mktime(&utc_tm);
        tz = (double)off / 3600.0;
    }

    for(int i = 0; i < 7; i++) {
        out_times[i] = in_times[i];
    }

    adjust_times(conf.latitude, conf.longitude, conf.elevation, tz, time,
                 out_times, c);

    return;
}

/* Return version of libprayertimes */
const char *prayertimes_version(void)
{
    return PRAYERTIMES_LIB_VER;
}
