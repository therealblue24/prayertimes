#include "prayertimes.h"
#include <stdio.h>

/* Welcome to C */

#ifndef M_PI
#define M_PI (3.141592653589793238462)
#endif /* M_PI */

/* Math macros */

#define num double

#define r2d(x)       (x * (180 / M_PI))
#define d2r(x)       (x * (M_PI / 180))
#define dsin(x)      (sin(d2r(x)))
#define dcos(x)      (cos(d2r(x)))
#define dtan(x)      (tan(d2r(x)))
#define dasin(x)     (r2d(asin(x)))
#define datan2(y, x) (r2d(atan2(y, x)))

/* bound angle to 0^ -> 360^ */
num bound_angle(num ang)
{
    return fmod(ang + 360, 360);
}

/* bound hr to 0h -> 24h */
num bound_hour(num hr)
{
    return fmod(hr + 24, 24);
}

/* calculate julian day number */
num calc_jdn(num y, num m, num d)
{
    /*
	From Wikipedia:
	JDN = (1461 × (Y + 4800 + (M − 14)/12))/4 +(367 × (M − 2 − 12 × ((M − 14)/12)))/12 − (3 × ((Y + 4900 + (M - 14)/12)/100))/4 + D − 32075
	*/
    const num term0 = (m - 14) / 12;
    const num term1 = y + 4800 + term0;
    const num term2 = term1 * 365.25;

    const num term3 = 12 * term0;
    const num term4 = m - 2 - term3;
    const num term5 = (367 * term4) / 12;
    const num term6 = term2 + term5;

    const num term7 = (y + 4900 + term0) / 100;
    const num term8 = term7 * 3;
    const num term9 = term8 / 4;

    const num term10 = d;
    const num term11 = 32075;
    return term6 - term9 + term10 - term11;
}

/* calculate julian day */
num calc_jd(num jdn, num hour, num minute, num second)
{
    return jdn + (hour - 12) / 24 + (minute) / 1440 + (second) / 86400;
}

/* julian day from unix timestamp */
num jd_now(time_t now)
{
    struct tm *t = gmtime(&now);
    num jdn_now = calc_jdn(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    num jd = calc_jd(jdn_now, t->tm_hour, t->tm_min, t->tm_sec);
    return jd;
}

/* julian day number from unix timestamp */
num jdn_now(time_t now)
{
    struct tm *t = gmtime(&now);
    num jdn_now = calc_jdn(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    return jdn_now;
}

/* julian day from struct tm */
num jd_tm(struct tm *t)
{
    num jdn_now = calc_jdn(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    num jd = calc_jd(jdn_now, t->tm_hour, t->tm_min, t->tm_sec);
    return jd;
}

/* unused i think */
num jdn_tm(struct tm *t)
{
    num jdn_now = calc_jdn(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    return jdn_now;
}

/* internal function - do NOT use */
void calc_core(num jd, num *_q, num *_RA, num *_e, num *_L)
{
    /* ref: http://praytimes.org/calculation */
    /* Should be accurate for the next 2 centures from 2000. */
    /* If it's 2200 by now, you should be using a better software. */
    num d = jd - 2451545.0;

    num g = bound_angle(357.529 + (0.98560028 * d));
    num q = bound_angle(280.459 + (0.98564736 * d));
    num L = bound_angle(q + (1.915 * dsin(g)) + (0.020 * dsin(2 * g)));

    num e = 23.439 - 0.00000036 * d;
    num RA = datan2(dcos(e) * dsin(L), dcos(L));
    RA = RA / 15;
    RA = bound_hour(RA);

    if(_q) {
        *_q = q;
    }
    if(_RA) {
        *_RA = RA;
    }
    if(_e) {
        *_e = e;
    }
    if(_L) {
        *_L = L;
    }

    return;
}

num eqt(num jd)
{
    num q, RA;
    calc_core(jd, &q, &RA, NULL, NULL);
    return (q / 15.0) - RA;
}

num dec(num jd)
{
    num e, L;
    calc_core(jd, NULL, NULL, &e, &L);
    return dasin(dsin(e) * dsin(L));
}

timelabel sun2norm(num suntime)
{
    timelabel ret = { 0 };
    num work = suntime;
    work = fmodf(work + 24, 24);
    ret.hour = floor(work);
    work -= floor(work);
    work *= 60;
    ret.minute = floor(work);
    work -= floor(work);
    work *= 60;
    ret.second = floor(work);
    work -= floor(work);
    work *= 1000;
    ret.millisecond = floor(work);
    return ret;
}

num angle_T(num a, [[maybe_unused]] num lng, num lat, num dec)
{
    /* ref: http://praytimes.org/calculation */
    const num upper = -dsin(a) - (dsin(lat) * dsin(dec));
    const num lower = dcos(lat) * dcos(dec);
    const num res = acos(upper / lower);
    return (1. / 15.) * r2d(res);
}

num acot(num x)
{
    return -atan(x) + (M_PI / 2);
}

num dacot(num x)
{
    return r2d(acot(x));
}

num angle_A(num n, num lat, [[maybe_unused]] num lng, num dec)
{
    /* ref: http://praytimes.org/calculation */
    num inner = dtan(lat - dec);
    num full = dacot(n + inner);
    num upper = dsin(full) - (dsin(lat) * dsin(dec));
    num lower = dcos(lat) * dcos(dec);
    return (1. / 15.) * r2d(acos(upper / lower));
}

static void print_time_12h_no_sec(const char *l, timelabel t)
{
    int pm = t.hour >= 12;
    if(pm && t.hour != 12) {
        t.hour -= 12;
    }
    if(t.hour == 0)
        t.hour = 12;
    const char *am_or_pm[] = { "AM", "PM" };
    printf("%s %2d:%02d %s\n", l, t.hour, t.minute, am_or_pm[pm]);
    return;
}

static void print_time_12h_sec(const char *l, timelabel t)
{
    int pm = t.hour >= 12;
    if(pm && t.hour != 12) {
        t.hour -= 12;
    }
    if(t.hour == 0)
        t.hour = 12;
    const char *am_or_pm[] = { "AM", "PM" };
    printf("%s %2d:%02d:%02d %s\n", l, t.hour, t.minute, t.second,
           am_or_pm[pm]);
    return;
}
static void print_time_24h_no_sec(const char *l, timelabel t)
{
    printf("%s %2d:%02d\n", l, t.hour, t.minute);
    return;
}

static void print_time_24h_sec(const char *l, timelabel t)
{
    printf("%s %2d:%02d:%02d\n", l, t.hour, t.minute, t.second);
    return;
}

void print_time(const char *l, timelabel t, print_conf_t pconf)
{
    if(pconf.am_pm && !pconf.seconds) {
        print_time_12h_no_sec(l, t);
        return;
    }
    if(pconf.am_pm && pconf.seconds) {
        print_time_12h_sec(l, t);
        return;
    }
    if(!pconf.am_pm && !pconf.seconds) {
        print_time_24h_no_sec(l, t);
        return;
    }
    if(!pconf.am_pm && pconf.seconds) {
        print_time_24h_sec(l, t);
        return;
    }
}

/* lat = latitude, lng = longitude, elev = elevation, Z = time zone,
   time = unix timestamp, times = pointer to array to store times, 
   asr_angle = asr angle */
void calc_schedule(num lat, num lng, num elev, num Z, time_t time, num *times,
                   double asr_angle)
{
    num jd = jdn_now(time);
    num eq_t = eqt(jd); // equation of time
    num decl = dec(jd); // declination of the sun
    num evfactor = 0.0347 * sqrt(elev); // elevation factor
    num dhuhr = 12 - eq_t + (Z - (lng / 15)); // dhuhr time

    const num fajr_angle = 13.5;
    const num isha_angle = 14.5;

    // Asr is when the shadow of an object is (angle)ths of its length
    // for shia: 2/7
    // for sunni: 1 || 2
    num asr_shadow = angle_A(asr_angle, lat, lng, decl);
    num asr = dhuhr + asr_shadow;
    // Fajr is an angle before dhuhr
    num fajr = dhuhr - angle_T(fajr_angle + evfactor, lng, lat, decl);
    // Sunrise is fajr with an angle of 5/6 degrees
    num sunrise = dhuhr - angle_T((5. / 6.) + evfactor, lng, lat, decl);
    // Sunset is dhuhr added on an angle of 29/12 degrees
    num sunset = dhuhr + angle_T((29. / 12.) + evfactor, lng, lat, decl);
    // Maghrib is an added few minutes after sunset for precaution
    num maghrib = sunset + (1. / 15.);
    // Isha is an angle after dhuhr
    num isha = dhuhr + angle_T(isha_angle + evfactor, lng, lat, decl);

    times[0] = fajr;
    times[1] = sunrise;
    times[2] = dhuhr;
    times[3] = asr;
    times[4] = sunset;
    times[5] = maghrib;
    times[6] = isha;
    return;
}
