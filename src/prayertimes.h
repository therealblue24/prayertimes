#ifndef PRAYER_TIMES_H_
#define PRAYER_TIMES_H_

#include <math.h>
#include <time.h>

typedef struct {
    int hour, minute, second;
} timelabel;

/* calculate julian day number */
double calc_jdn(double y, double m, double d);
/* calculate julian day */
double calc_jd(double jdn, double hour, double minute, double second);
/* calculate julian day given timestamp */
double jd_now(time_t now);
/* calculate julian day given struct tm */
double jd_tm(struct tm *t);
/* equation of time */
double eqt(double jd);
/* sun declination */
double dec(double jd);
/* T angle function */
double angle_T(double a, double lng, double lat, double dec);
/* A angle function */
double angle_A(double n, double lng, double lat, double dec);
/* calculate a prayer time schedule, given:
 * lat: latitude
 * lng: longitude
 * elev: elevation (in meters)
 * Z: timezone
 * time: unix timestamp
 * times: array to store prayer times in
 * asr_angle: angle for asr
 */
void calc_schedule(double lat, double lng, double elev, double Z, time_t time,
                   double *times, double asr_angle);
/* print a time in AM/PM format */
void print_time(const char *l, timelabel t);

/* suntime -> timelabel */
timelabel sun2norm(double suntime);

#endif /* #ifndef PRAYER_TIMES_H_ */
