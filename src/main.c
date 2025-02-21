#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <assert.h>
#include "prayertimes.h"

#define VERSION "1.4"

#define _str(x)       x
#define xstr(x)       _str(#x)
#define xstrcat(a, b) xstr(a) xstr(b)

#ifndef __APPLE__
/* too lazy to impl */
#define strlcpy strncpy
#define strlcat strncat
#endif /* __APPLE__ */

static const char *prefixes[] = {
    [1] = "st",  [2] = "nd",  [3] = "rd",  [4] = "th",  [5] = "th",
    [6] = "th",  [7] = "th",  [8] = "th",  [9] = "th",  [10] = "th",
    [11] = "th", [12] = "th", [13] = "th", [14] = "th", [15] = "th",
    [16] = "th", [17] = "th", [18] = "th", [19] = "th", [20] = "th",
    [21] = "st", [22] = "nd", [23] = "rd", [24] = "th", [25] = "th",
    [26] = "th", [27] = "th", [28] = "th", [29] = "th", [30] = "th",
    [31] = "st", NULL
};

static const char *months[] = { "January",   "Feburary", "March",    "April",
                                "May",       "June",     "July",     "August",
                                "September", "October",  "November", "December",
                                NULL };
static const char *weekdays[] = { "Sunday",   "Monday", "Tuesday",  "Wednesday",
                                  "Thursday", "Friday", "Saturday", NULL };

void zfgets(char *buf, int size, FILE *f)
{
    memset(buf, 0, size);
    char *v = fgets(buf, size, f);
    if(!v) {
        fprintf(stderr, "ERROR: Failed to read stdin: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < size; i++) {
        if(buf[i] == '\n' || buf[i] == '\r') {
            buf[i] = '\0';
        }
    }
    return;
}

void init_conf(char *path)
{
    char b[512] = { 0 };
    double lat = 0, lng = 0, elev = 0, ang = 0;
    printf("Prayer time configurer\n");
    printf("Latitude: ");
    zfgets(b, sizeof(b), stdin);
    lat = strtod(b, NULL);
    printf("Longitude: ");
    zfgets(b, sizeof(b), stdin);
    lng = strtod(b, NULL);
    printf("Elevation in meters (enter 0 if you don't know): ");
    zfgets(b, sizeof(b), stdin);
    elev = strtod(b, NULL);
    printf("Shia or Sunni?: ");
    zfgets(b, sizeof(b), stdin);
    for(int i = 0; i < 512; i++) {
        if(b[i] == '\n' || b[i] == '\r') {
            b[i] = 0;
        }
        if(b[i])
            b[i] = tolower(b[i]);
    }
    if(strcmp(b, "shia") == 0) {
        ang = 2. / 7.;
    } else if(strcmp(b, "sunni") == 0) {
label0:;
        printf("Shafi'i (1) or Hanafi (2) ?: ");
        zfgets(b, sizeof(b), stdin);
        ang = strtod(b, NULL);
        if(ang != 1 || ang != 2) {
            /* the legend of sisyphus */
            goto label0;
        }
    } else {
        printf("Please enter 'Shia' or 'Sunni'.\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok, writing binary config file... \n");

    FILE *f = fopen(path, "wb");
    if(!f) {
        fprintf(stderr, "ERROR: Failed to open config file: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* who cares you are not spreading this config file */
    assert(fwrite(&lng, sizeof(double), 1, f));
    assert(fwrite(&lat, sizeof(double), 1, f));
    assert(fwrite(&ang, sizeof(double), 1, f));
    /* seperated so that it doesn't ruin alignment */
    assert(fwrite(&elev, sizeof(double), 1, f));

    assert(fflush(f) == 0);

    fclose(f); /* swag */
    putchar('\n');
}

void copyright(void)
{
    printf(
        "therealblue24's prayer time calculator\nCopyright (C) 2025 therealblue24 (under the MIT license).\n");
    return;
}

void show_version(void)
{
    printf("version " VERSION ", " __DATE__ "\n");
    return;
}

void show_help(bool version)
{
    copyright();
    if(version) {
        show_version();
    }
    putchar('\n');
    printf("Usage:\n");
    printf("\t-s, --silent\t\t\tonly print times\n");
    printf("\t-f, --show-future-only\t\tshow only future times\n");
    printf("\t-rc, --reconfigure\t\treconfigure location, method\n");
    printf("\t-ss, --sunset\t\t\tprint sunset time\n");
    printf("\t--imsak\t\t\t\tprint imsak time\n");
    printf("\t--midnight\t\t\tprint midnight\n");
    printf("\t-u, --utc\t\t\tprint times in UTC\n");
    printf("\t-h, --help\t\t\tthis page\n");
    printf("\t--version\t\t\tprint version of prayertimes\n");
    printf(
        "\t-c, --color\t\t\tcolorize prayer times (requires truecolor support)\n");
    printf("\t--no-fajr\t\t\tdon't print fajr\n");
    printf("\t--no-sunrise\t\t\tdon't print sunrise\n");
    printf("\t--no-dhuhr\t\t\tdon't print dhuhr\n");
    printf("\t--no-asr\t\t\tdon't print asr\n");
    printf("\t--no-maghrib\t\t\tdon't print maghrib\n");
    printf("\t--no-isha\t\t\tdon't print isha\n");
    printf("\t--usage\t\t\t\tthis page\n");
    printf("\t-12h\t\t\t\tprint times in 12 hour format\n");
    printf("\t-24h\t\t\t\tprint times in 24 hour format\n");
    printf("\t--seconds\t\t\tprint seconds along with time\n");
    printf("\t-fa, --fajr-angle\t\tset fajr angle\n");
    printf("\t-aa, --asr-angle\t\tset custom asr angle\n");
    printf("\t-ia, --isha-angle\t\tset isha angle\n");
    printf("\t-mm, --maghrib-minutes\t\tset maghrib minutes\n");
    printf("\t-im, --imsak-minutes\t\tset imsak minutes\n");
    printf("\t--adjust\t\t\tadjust prayer times\n");
}

int main(int argc, char *argv[])
{
    print_conf_t pconf = { .am_pm = true, .seconds = false, .color = false };
    struct {
        bool silent_mode; /* only print times */
        bool show_future_only; /* only show future times */
        bool reconf; /* reconfigure? */
        bool print_sunset; /* print sunset */
        bool utc; /* use UTC */
        bool help; /* help */
        bool version; /* version */
        bool no_show[7]; /* dont show certain time */
        times_conf timeconf; /* time configuration */
        bool imsak; /* print imsak */
        bool midnight; /* print midnight */
        double imsak_minutes; /* minutes for imsak */
        bool angset; /* angle set */
        bool shia; /* autodetected */
        bool sunni; /* autodetected */
        bool adjust; /* adjust prayer times */
    } conf = {
        .silent_mode = false,
        .show_future_only = false,
        .reconf = false,
        .print_sunset = false,
        .utc = false,
        .help = false,
        .midnight = false,
        .imsak = false,
        .imsak_minutes = 15.0,
        .timeconf = { .asr_angle = 0,
                     .fajr_angle = 13.5,
                     .isha_angle = 14.5,
                     .maghrib_minutes = 15 },
        .angset = false,
        .shia = false,
        .sunni = false,
        .adjust = false
    };

    int argc2 = 1;
    while(argc2 < argc) {
        const char *arg = argv[argc2];
        if(strcmp(arg, "-12h") == 0) {
            pconf.am_pm = true;
        }
        if(strcmp(arg, "-24h") == 0) {
            pconf.am_pm = false;
        }
        if(strcmp(arg, "--seconds") == 0) {
            pconf.seconds = true;
        }
        if(strcmp(arg, "-s") == 0 || strcmp(arg, "--silent") == 0) {
            conf.silent_mode = true;
        }
        if(strcmp(arg, "-f") == 0 || strcmp(arg, "--show-future-only") == 0) {
            conf.show_future_only = true;
        }
        if(strcmp(arg, "--reconf") == 0 || strcmp(arg, "-rc") == 0) {
            conf.reconf = true;
        }
        if(strcmp(arg, "-ss") == 0 || strcmp(arg, "--sunset") == 0) {
            conf.print_sunset = true;
        }
        if(strcmp(arg, "-u") == 0 || strcmp(arg, "--utc") == 0) {
            conf.utc = true;
        }
        if(strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 ||
           strcmp(arg, "-?") == 0 || strcmp(arg, "?") == 0 ||
           strcmp(arg, "--usage") == 0) {
            conf.help = true;
        }
        if(strcmp(arg, "--version") == 0) {
            conf.version = true;
        }
        if(strcmp(arg, "-c") == 0 || strcmp(arg, "--color") == 0) {
            pconf.color = true;
        }
        if(strcmp(arg, "-fa") == 0 || strcmp(arg, "--fajr-angle") == 0) {
            double fajr_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.fajr_angle = fajr_angle;
        }
        if(strcmp(arg, "-aa") == 0 || strcmp(arg, "--asr-angle") == 0) {
            double asr_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.asr_angle = asr_angle;
            conf.angset = true;
        }
        if(strcmp(arg, "-ia") == 0 || strcmp(arg, "--isha-angle") == 0) {
            double isha_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.isha_angle = isha_angle;
        }
        if(strcmp(arg, "-mm") == 0 || strcmp(arg, "--maghrib-minutes") == 0) {
            double maghrib_minutes = strtod(argv[++argc2], NULL);
            conf.timeconf.maghrib_minutes = maghrib_minutes;
        }
        if(strcmp(arg, "--imsak") == 0) {
            conf.imsak = true;
        }
        if(strcmp(arg, "--midnight") == 0) {
            conf.midnight = true;
        }
        if(strcmp(arg, "-im") == 0 || strcmp(arg, "--imsak-minutes") == 0) {
            double imsak_minutes = strtod(argv[++argc2], NULL);
            conf.imsak_minutes = imsak_minutes;
        }
        if(strcmp(arg, "--adjust") == 0) {
            conf.adjust = true;
        }
#define map(n, l)                             \
    if(strcmp(arg, xstrcat(--no, -n)) == 0) { \
        conf.no_show[l] = true;               \
    }
        map(fajr, 0);
        map(sunrise, 1);
        map(dhuhr, 2);
        map(asr, 3);
        /* map(sunset, 4); */
        map(maghrib, 5);
        map(isha, 6);
#undef map
        argc2++;
    }
    if(conf.help) {
        show_help(conf.version);
        return 0;
    }
    if(conf.version) {
        copyright();
        show_version();
        return 0;
    }
    if(!conf.silent_mode) {
        copyright();
        printf("to reconfigure, please run '%s --reconf'\n\n", argv[0]);
    }

    char *home = getenv("HOME");
    if(!home) {
        fprintf(stderr, "ERROR: failed to obtain $HOME: %s\n", strerror(errno));
        return 1;
    }

    bool configured = false;

    char *dpath = calloc(1, 1024);
    char *rpath = calloc(1, 1024);
    if(!dpath || !rpath) {
        fprintf(stderr, "ERROR: Failed to allocate memory. ENOMEM?: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    strlcpy(rpath, home, 1024);
    strlcpy(dpath, home, 1024);
    strlcat(dpath, "/.config/prayertimes", 1024);
    strlcat(rpath, "/.config/prayertimes/config.dat", 1024);

    if(access(dpath, F_OK) == -1) {
        if(errno == ENOENT) {
            printf("Creating config directory\n");
            if(mkdir(dpath, 0755) == -1) {
                fprintf(stderr, "ERROR: unable to create %s: %s\n", dpath,
                        strerror(errno));
                return 1;
            }
        } else {
            fprintf(stderr, "ERROR: unable to access %s: %s\n", dpath,
                    strerror(errno));
            return 1;
        }
    }

    if(access(rpath, F_OK) == -1) {
        if(errno == ENOENT) {
            printf("Configuring for first use\n");
            if(!configured)
                init_conf(rpath);
            configured = true;
        } else {
            fprintf(stderr, "ERROR: unable to access %s: %s\n", rpath,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if(!configured && conf.reconf) {
        init_conf(rpath);
        configured = true;
    }

    double lat, lng, elev, ang = 0;
    time_t now = time(NULL);
    double now_suntime = suntime_now(now);

    FILE *f = fopen(rpath, "rb");
    if(!f) {
        fprintf(stderr, "ERROR: Failed to open configuration file: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    assert(fread(&lng, sizeof(double), 1, f));
    assert(fread(&lat, sizeof(double), 1, f));
    assert(fread(&ang, sizeof(double), 1, f));
    /* seperated so that it doesn't ruin alignment */
    assert(fread(&elev, sizeof(double), 1, f));

    if(!conf.angset) {
        conf.timeconf.asr_angle = ang;
    }

    const double shia_asr_angle = (double)2 / (double)7;

    /* Simple shia/sunni detection */
    if(conf.timeconf.asr_angle == shia_asr_angle) {
        conf.shia = true;
    } else {
        conf.sunni = true;
    }

    fclose(f); /* swag */

    free(rpath);
    free(dpath);

    struct tm tm = *localtime(&now);

    long off = 0;
    const char *zone = NULL;
#ifdef __APPLE__
    off = tm.tm_gmtoff;
    zone = tm.tm_zone;
#else
    /* Linux doesn't have tm.tm_gmtoff and tm.tm_zone. Why??????? */
    struct tm utc_tm = *gmtime(&now);
    off = mktime(&tm) - mktime(&utc_tm);
#endif /* __APPLE__ */

    double Z = (double)off / 3600.0;
    if(conf.utc) {
        Z = 0;
    }
    now_suntime = fmod(now_suntime + Z + 24, 24.0);
    if(zone && !conf.silent_mode) {
        printf("Time zone: %g hours (%s)\n", Z, zone);
    } else if(!conf.silent_mode) {
        printf("Time zone: %g hours (zone N/A)\n", Z);
    }

    if(!conf.silent_mode) {
        printf("Prayer times for %s, %s %d%s, %d:\n", weekdays[tm.tm_wday],
               months[tm.tm_mon], tm.tm_mday, prefixes[tm.tm_mday],
               tm.tm_year + 1900);
    }
    double t[7] = { 0 };

    calc_schedule(lat, lng, elev, Z, now, t, conf.timeconf);
    if(conf.adjust) {
        adjust_times(lat, lng, elev, Z, now, t, conf.timeconf);
    }
    double fajr_suntime = t[0];
    double sunrise_suntime = t[1];
    double dhuhr_suntime = t[2];
    double asr_suntime = t[3];
    double sunset_suntime = t[4];
    double maghrib_suntime = t[5];
    double isha_suntime = t[6];

    double imsak_suntime = fajr_suntime - (conf.imsak_minutes / 60);
    double midnight_suntime = 0;

    if(conf.shia) {
        midnight_suntime = bound_hour((sunset_suntime + fajr_suntime + 24) / 2);
    } else if(conf.sunni) {
        midnight_suntime =
            bound_hour((sunset_suntime + sunrise_suntime + 24) / 2);
    }

    timelabel fajr = sun2norm(fajr_suntime);
    timelabel sunrise = sun2norm(sunrise_suntime);
    timelabel dhuhr = sun2norm(dhuhr_suntime);
    timelabel asr = sun2norm(asr_suntime);
    timelabel sunset = sun2norm(sunset_suntime);
    timelabel maghrib = sun2norm(maghrib_suntime);
    timelabel isha = sun2norm(isha_suntime);
    timelabel imsak = sun2norm(imsak_suntime);
    timelabel midnight = sun2norm(midnight_suntime);

#define X(s, c, e)                             \
    if(conf.show_future_only && (c) && (e)) {  \
        s;                                     \
    } else if(!conf.show_future_only && (e)) { \
        s;                                     \
    }
#define Y(n) (!conf.no_show[n])

    X(print_time("Imsak:   ", imsak, pconf, 0), now_suntime < imsak_suntime,
      conf.imsak);
    X(print_time("Fajr:    ", fajr, pconf, 0), now_suntime < fajr_suntime,
      Y(0));
    X(print_time("Sunrise: ", sunrise, pconf, 1), now_suntime < sunrise_suntime,
      Y(1));
    X(print_time("Dhuhr:   ", dhuhr, pconf, 2), now_suntime < dhuhr_suntime,
      Y(2));
    X(print_time("Asr:     ", asr, pconf, 3), (now_suntime < asr_suntime),
      Y(3));
    X(print_time("Sunset:  ", sunset, pconf, 4),
      (now_suntime < sunset_suntime) && conf.print_sunset,
      conf.print_sunset && Y(4));
    X(print_time("Maghrib: ", maghrib, pconf, 5), now_suntime < maghrib_suntime,
      Y(5));
    X(print_time("Isha:    ", isha, pconf, 6), (now_suntime < isha_suntime),
      Y(6));
    if(conf.midnight) {
        putchar('\n');
    }
    X(print_time("Midnight: ", midnight, pconf, 7),
      now_suntime < midnight_suntime, conf.midnight);

#undef X
#undef Y
}
