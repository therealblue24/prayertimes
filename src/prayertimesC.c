#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */
#include "prayertimesC.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* math.h doesn't have M_PI on Linux. Why??? */

#ifndef M_PI
#define M_PI (3.141592653589793238462)
#endif /* M_PI */

/* Math macros */

/* you will not stop me */
#define num double

#define r2d(x)       (x * (180 / M_PI))
#define d2r(x)       (x * (M_PI / 180))
#define dsin(x)      (sin(d2r(x)))
#define dcos(x)      (cos(d2r(x)))
#define dtan(x)      (tan(d2r(x)))
#define dasin(x)     (r2d(asin(x)))
#define datan2(y, x) (r2d(atan2(y, x)))

/* bound angle to 0^ -> 360^ */
APIFN num bound_angle(num ang)
{
    return fmod(ang + 360, 360);
}

/* bound hr to 0h -> 24h */
APIFN num bound_hour(num hr)
{
    return fmod(hr + 24, 24);
}

APIFN num jdn_now_with_timezone(time_t now, time_t off)
{
    time_t n = now;
    /* main thing */

    /* Thanks to Wikipedia (https://en.wikipedia.org/wiki/Julian_day#Variants)
     * for this clever trick! */
    n += off; /* adjust to native timezone */
    n -= (n % 86400); /* beginning of day */
    num jd = (num)n;
    jd = ((double)n / 86400.) + 2440587.5; /* timestamp -> JDN */
    return jd;
}

/* julian day number from unix timestamp */
/* unused.. for now.. */
APIFN num jdn_now(time_t now)
{
    time_t n = now;
    /* timezone detection */

    struct tm tm = *localtime(&n);
    /* Linux doesn't have tm.tm_gmtoff and tm.tm_zone. Why??????? */
    struct tm utc_tm = *gmtime(&now);
    /* Thanks
     * https://stackoverflow.com/questions/13804095/get-the-time-zone-gmt-offset-in-c!
     */
    utc_tm.tm_isdst = -1;
    long off = mktime(&tm) - mktime(&utc_tm);
    return jdn_now_with_timezone(now, off);
}

/* suntime from unix timestamp */
APIFN num suntime_now(time_t now)
{
    time_t d = now % 86400;
    return ((num)d / 3600.0);
}

/* internal function - do NOT use */
APIFN static inline void calc_core(num jd, num *_q, num *_ra, num *_e, num *_l)
{
    /* ref: http://praytimes.org/calculation */
    /* Should be accurate for the next 2 centures from 2000. */
    /* If it's 2200 by now, you should be using better software. */
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
    if(_ra) {
        *_ra = RA;
    }
    if(_e) {
        *_e = e;
    }
    if(_l) {
        *_l = L;
    }

    return;
}

/* calculate equation of time given julian day */
APIFN num eqt(num jd)
{
    num q, RA;
    calc_core(jd, &q, &RA, NULL, NULL);
    return (q / 15.0) - RA;
}

/* calculate solar declination given julian day */
APIFN num dec(num jd)
{
    num e, L;
    calc_core(jd, NULL, NULL, &e, &L);
    return dasin(dsin(e) * dsin(L));
}

/* turn a suntime into a normal time */
APIFN timelabel sun2norm(num suntime)
{
    timelabel ret = { 0 };
    num work = suntime, tmp = 0;
    work = fmod(work + 24, 24);
    tmp = floor(work);
    ret.hour = (int)tmp;
    work -= floor(work);
    work *= 60;
    tmp = floor(work);
    ret.minute = (int)tmp;
    work -= floor(work);
    work *= 60;
    tmp = floor(work);
    ret.second = (int)tmp;
    work -= floor(work);
    work *= 1000;
    tmp = floor(work);
    ret.millisecond = (int)tmp;
    return ret;
}

/* T angle function */
APIFN num angle_T(num a, __attribute__((unused)) num lng, num lat, num dec)
{
    /* ref: http://praytimes.org/calculation */
    const num upper = -dsin(a) - (dsin(lat) * dsin(dec));
    const num lower = dcos(lat) * dcos(dec);
    const num res = acos(upper / lower);
    return (1. / 15.) * r2d(res);
}

/* arccotangent */
static APIFN num acot(num x)
{
    return atan(1 / x);
}

/* degree arccotangent */
static APIFN num dacot(num x)
{
    return r2d(acot(x));
}

/* A angle function, for Asr */
APIFN num angle_A(num n, num lat, num lng, num dec)
{
    /* ref: http://praytimes.org/calculation */
    /* And also inspired from C++ impl in http://praytimes.org/wiki/Code */
    /* And also thanks to Wikipedia for making me realize I did
     * longitude - declination
     * rather than latitude - declination.
     * :facepalm:.                         */
    num abs_angle = fabs(lat - dec);
    num tangent = (dtan(abs_angle));
    num angle = -dacot(n + tangent);
    return angle_T(angle, lng, lat, dec);
}

static APIFN void print_time_12h_no_sec(const char *l, timelabel t, char **b)
{
    int pm = t.hour >= 12;
    if(pm && t.hour != 12) {
        t.hour -= 12;
    }
    if(t.hour == 0)
        t.hour = 12;
    const char *am_or_pm[] = { "AM", "PM" };
    asprintf(b, "%s %2" PRId32 ":%02" PRId32 " %s\n", l, t.hour, t.minute,
             am_or_pm[pm]);
    return;
}

static APIFN void print_time_12h_sec(const char *l, timelabel t, char **b)
{
    int pm = t.hour >= 12;
    if(pm && t.hour != 12) {
        t.hour -= 12;
    }
    if(t.hour == 0)
        t.hour = 12;
    const char *am_or_pm[] = { "AM", "PM" };
    asprintf(b, "%s %2" PRId32 ":%02" PRId32 ":%02" PRId32 " %s\n", l, t.hour,
             t.minute, t.second, am_or_pm[pm]);
    return;
}

static APIFN void print_time_24h_no_sec(const char *l, timelabel t, char **b)
{
    asprintf(b, "%s %2" PRId32 ":%02" PRId32 "\n", l, t.hour, t.minute);
    return;
}

static APIFN void print_time_24h_sec(const char *l, timelabel t, char **b)
{
    asprintf(b, "%s %2" PRId32 ":%02" PRId32 ":%02" PRId32 "\n", l, t.hour,
             t.minute, t.second);
    return;
}

/* RGB color */
typedef struct color {
    uint8_t r, g, b;
} color;

/* A pair of 2 RGB colors */
typedef struct colorp {
    color b, e;
} colorp;

static const colorp timecolor[8] = {
    /* All prefixed with FINDME: so that I can search for them easily. */
    /* FINDME:fajr    */
    { { 227, 223, 211 }, { 243, 245, 149 } },
    /* FINDME:sunrise */
    { { 255, 68, 41 },   { 252, 222, 114 } },
    /* FINDME:dhuhr   */
    { { 7, 118, 245 },   { 7, 245, 118 }   },
    /* FINDME:asr     */
    { { 7, 245, 118 },   { 190, 129, 240 } },
    /* FINDME:sunset  */
    { { 252, 222, 114 }, { 255, 68, 41 }   },
    /* FINDME:maghrib */
    { { 98, 33, 252 },   { 245, 98, 7 }    },
    /* FINDME:isha    */
    { { 245, 98, 7 },    { 75, 68, 92 }    },
    /* FINDME:midnight*/
    { { 255, 255, 255 }, { 128, 128, 128 } },
};

/* mix 2 colors */
static APIFN color mix(colorp s, int i, int l)
{
    float p2 = ((float)i) / ((float)(l - 1));
    float p1 = 1 - p2;
    float r = (p1 * s.b.r) + (p2 * s.e.r);
    float g = (p1 * s.b.g) + (p2 * s.e.g);
    float b = (p1 * s.b.b) + (p2 * s.e.b);
    r *= 1.2f;
    g *= 1.2f;
    b *= 1.2f;
    if(r > 255)
        r = 255;
    if(g > 255)
        g = 255;
    if(b > 255)
        b = 255;
    if(r < 0)
        r = 0;
    if(g < 0)
        g = 0;
    if(b < 0)
        b = 0;
    return (color){ (uint8_t)r, (uint8_t)g, (uint8_t)b };
}

static APIFN void setcol(color c)
{
    printf("\033[38;2;%" PRId32 ";%" PRId32 ";%" PRId32 "m", c.r, c.g, c.b);
}

APIFN void print_time(const char *l, timelabel t, print_conf_t pconf, int time)
{
    char *b = NULL;
    if(pconf.am_pm && !pconf.seconds) {
        print_time_12h_no_sec(l, t, &b);
    }
    if(pconf.am_pm && pconf.seconds) {
        print_time_12h_sec(l, t, &b);
    }
    if(!pconf.am_pm && !pconf.seconds) {
        print_time_24h_no_sec(l, t, &b);
    }
    if(!pconf.am_pm && pconf.seconds) {
        print_time_24h_sec(l, t, &b);
    }
    if(!b) {
        printf("ERROR: Failed to print time\n");
        exit(EXIT_FAILURE);
    }
    if(!pconf.color) {
        printf("%s", b);
        free(b);
        return;
    }
    colorp colp = timecolor[time];
    color c;
    int len = (int)strlen(b);
    int rl = 0;
    for(int i = 0; i < len; i++) {
        rl += b[i] != ' ';
    }
    int p = 0;
    for(int i = 0; i < len; i++) {
        c = mix(colp, p, rl - 1);
        setcol(c);
        putchar(b[i]);
        p += b[i] != ' ';
    }
    printf("\033[0m");

    free(b);
    return;
}

/* lat = latitude, lng = longitude, elev = elevation, Z = time zone,
   time = unix timestamp, times = pointer to array to store times,
   conf = configuration */

/* Default conf is:
 * asr_shadow_length: <depends>
 * fajr_angle: 13.5^
 * isha_angle: 14.5^
 * maghrib_minutes: 15 min
 */

enum { FAJR = 0, SUNRISE, DHUHR, ASR, SUNSET, MAGHRIB, ISHA };

/* Dhuhr */
static APIFN num midday(num jd, num lng, num Z)
{
    return 12 - eqt(jd) + (Z - (lng / 15));
}

/* calculate a prayer schedule */
APIFN void calc_schedule(num lat, num lng, num elev, num Z, time_t time,
                         num *times, times_conf conf)
{
    num jd = jdn_now_with_timezone(time, (time_t)(3600 * Z));
    num decl = dec(jd); // declination of the sun
    num evfactor = 0.0347 * sqrt(elev); // elevation factor
    num dhuhr = midday(jd, lng, Z); // dhuhr time

    // Asr is when the shadow of an object is (angle)ths of its length
    // for shia: 2/7
    // for sunni: 1 || 2
    num asr_shadow = angle_A(conf.asr_shadow_length, lat, lng, decl);
    num asr = dhuhr + asr_shadow;
    // Fajr is an angle before dhuhr
    num fajr = dhuhr - angle_T(conf.fajr_angle + evfactor, lng, lat, decl);
    // Sunrise is dhuhr with an angle of 5/6 degrees
    num sunrise = dhuhr - angle_T((5. / 6.) + evfactor, lng, lat, decl);
    // Sunset is dhuhr added on an angle of 5/6 degrees
    num sunset = dhuhr + angle_T((5. / 6.) + evfactor, lng, lat, decl);
    // Maghrib is either some minutes after sunset or some minutes after an
    // angle
    num maghrib = sunset + (conf.maghrib_minutes / 60);
    if(conf.use_maghrib_angle) {
        maghrib = dhuhr +
                  angle_T(conf.maghrib_angle + evfactor, lng, lat, decl) +
                  (conf.maghrib_minutes / 60);
    }
    // Isha is an angle after dhuhr + minutes
    num isha = dhuhr + angle_T(conf.isha_angle + evfactor, lng, lat, decl) +
               (conf.isha_minutes / 60);

    if(!conf.use_isha_angle) {
        isha = maghrib + (conf.isha_minutes / 60);
    }

    times[FAJR] = fajr;
    times[SUNRISE] = sunrise;
    times[DHUHR] = dhuhr;
    times[ASR] = asr;
    times[SUNSET] = sunset;
    times[MAGHRIB] = maghrib;
    times[ISHA] = isha;
    return;
}

/* Adjust times, inspired from C++ impl in http://praytimes.org/wiki/Code */
APIFN void adjust_times(num lat, num lng, num elev, num Z, time_t time,
                        num *times, times_conf conf)
{
    /* Fix times to represent true values */
    times[MAGHRIB] -= (conf.maghrib_minutes / 60);
    times[ISHA] -= (conf.isha_minutes / 60);
    /* suntime -> day percent */
    for(int i = 0; i < 7; i++) {
        times[i] = bound_hour(times[i]) / 24;
    }

    num jd = jdn_now_with_timezone(time, (time_t)(3600 * Z));
    num evfactor = 0.0347 * sqrt(elev); // elevation factor
    num dhuhr = midday(jd + times[DHUHR], lng, Z);

    num fajr =
        midday(jd + times[FAJR], lng, Z) -
        angle_T(conf.fajr_angle + evfactor, lng, lat, dec(jd + times[FAJR]));
    num isha =
        midday(jd + times[ISHA], lng, Z) +
        angle_T(conf.isha_angle + evfactor, lng, lat, dec(jd + times[ISHA])) +
        (conf.isha_minutes / 60);

    num sunrise =
        midday(jd + times[SUNRISE], lng, Z) -
        angle_T((5. / 6.) + evfactor, lng, lat, dec(jd + times[SUNRISE]));
    num sunset =
        midday(jd + times[SUNSET], lng, Z) +
        angle_T((5. / 6.) + evfactor, lng, lat, dec(jd + times[SUNSET]));

    num maghrib = sunset + (conf.maghrib_minutes / 60);

    if(conf.use_maghrib_angle) {
        maghrib = midday(jd + times[MAGHRIB], lng, Z) +
                  angle_T(conf.maghrib_angle + evfactor, lng, lat,
                          dec(jd + times[MAGHRIB])) +
                  (conf.maghrib_minutes / 60);
    }

    if(!conf.use_isha_angle) {
        isha = maghrib + (conf.isha_minutes / 60);
    }

    num asr_shadow =
        angle_A(conf.asr_shadow_length, lat, lng, dec(jd + times[ASR]));
    num asr = midday(jd + times[ASR], lng, Z) + asr_shadow;

    times[FAJR] = fajr;
    times[SUNRISE] = sunrise;
    times[DHUHR] = dhuhr;
    times[ASR] = asr;
    times[SUNSET] = sunset;
    times[MAGHRIB] = maghrib;
    times[ISHA] = isha;

    return;
}
