#ifndef PRAYER_TIMES_H_
#define PRAYER_TIMES_H_

#include <math.h>
#include <time.h>

typedef struct {
    int hour, minute, second;
} timelabel;

double calc_jdn(double y, double m, double d);
double calc_jd(double jdn, double hour, double minute, double second);
double jd_now(time_t now);
double jd_tm(struct tm *t);
double eqt(double jd);
double dec(double jd);
double angle_T(double a, double lng, double lat, double dec);
double angle_A(double n, double lng, double lat, double dec);
void calc_schedule(double lat, double lng, double elev, double Z, time_t time,
                   double *times, double asr_angle);
void print_time(const char *l, timelabel t);

timelabel sun2norm(double suntime);

#endif /* #ifndef PRAYER_TIMES_H_ */
