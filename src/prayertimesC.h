#ifndef PRAYER_TIMES_PRIVATE_H_
#define PRAYER_TIMES_PRIVATE_H_

#include <math.h>
#include <stdbool.h>
#include <time.h>

#define APIFN __attribute__((visibility("hidden")))

typedef struct {
    int hour, minute, second, millisecond;
} timelabel;

#define PRAYERTIMES_LIB_VER "1.0.2"

/* give current suntime given timestamp */
double APIFN suntime_now(time_t now);
/* equation of time */
double APIFN eqt(double jd);
/* sun declination */
double APIFN dec(double jd);
/* T angle function */
double APIFN angle_T(double a, double lng, double lat, double dec);
/* A angle function */
double APIFN angle_A(double n, double lng, double lat, double dec);
/* fix angle */
double APIFN bound_angle(double ang);
/* fix suntime */
double APIFN bound_hour(double hr);
/* get JDN */
double APIFN jdn_now(time_t t);
/* get JDN but special */
double APIFN jdn_now_with_timezone(time_t t, time_t off);

typedef struct {
    double asr_shadow_length; /* shadow length of Asr */
    double fajr_angle, isha_angle; /* angles of Fajr and Isha */
    double
        maghrib_minutes; /* how much minutes after [sunset/calculated maghrib] is maghrib */
    double isha_minutes; /* how much minutes after [calculated isha] is isha */
    double maghrib_angle; /* maghrib angle */
    bool use_maghrib_angle; /* use maghrib angle */
    bool use_isha_angle; /* use isha angle */

    /* adjustment method */
    enum { MIDDLE_NIGHT = 0, ONE_SEVENTH_NIGHT, ANGLE_BASED } adjust_method;
} times_conf;

/* calculate a prayer time schedule, given:
 * lat: latitude
 * lng: longitude
 * elev: elevation (in meters)
 * Z: timezone
 * time: unix timestamp
 * times: array to store prayer times in
 * conf: configuration
 */
void APIFN calc_schedule(double lat, double lng, double elev, double Z,
                         time_t time, double *times, times_conf conf);

/* adjust times, same args as calc_schedule */
void APIFN adjust_times(double lat, double lng, double elev, double Z,
                        time_t time, double *times, times_conf conf);

typedef struct print_conf {
    bool am_pm; /* AM/PM time? */
    bool seconds; /* include seconds */
    bool color; /* c o l o r */
} print_conf_t;

/* print a time */
void APIFN print_time(const char *l, timelabel t, print_conf_t conf, int time);

/* suntime -> timelabel */
timelabel APIFN sun2norm(double suntime);

#endif /* #ifndef PRAYER_TIMES_PRIVATE_H_ */
