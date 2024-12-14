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
#include "prayertimes.h"

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

void init_conf(char *path)
{
    double lat = 0, lng = 0, elev = 0, ang = 0;
    printf("Prayer time configurer\n");
    printf("Latitude: ");
    fscanf(stdin, "%lf", &lat);
    printf("Longitude: ");
    fscanf(stdin, "%lf", &lng);
    printf("Elevation in meters (enter 0 if you don't know): ");
    fscanf(stdin, "%lf", &elev);
    char b[512] = { 0 };
    printf("Shia or Sunni?: ");
    fscanf(stdin, "%s", b);
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
        fscanf(stdin, "%lf", &ang);
        if(ang != 1 || ang != 2) {
            /* the legend of sisyphus */
            goto label0;
        }
    } else {
        printf("Please enter 'Shia' or 'Sunni'.\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok, writing binary config file... \n");

    /* Stupid workaround to fix a segfault. I don't know how to solve the bug properly. */
    FILE *f = fopen(path, "wb");
    /* who cares you are not spreading this config file */
    fwrite(&lng, sizeof(double), 1, f);
    fwrite(&lat, sizeof(double), 1, f);
    fwrite(&ang, sizeof(double), 1, f);
    /* seperated so that it doesn't ruin alignment */
    fwrite(&elev, sizeof(double), 1, f);

    fflush(f);

    fclose(f); /* swag */
    putchar('\n');
}

void show_help(void)
{
    printf(
        "therealblue24's prayer time calculator\nCopyright (C) 2024 therealblue24 (under MIT license).\n\n");
    printf("Usage:\n");
    printf("\t-s, --silent\t\t\tonly print times\n");
    printf("\t-f, --show-future-only\t\tshow only future times\n");
    printf("\t-rc, --reconfigure\t\treconfigure location, method\n");
    printf("\t-ss, --sunset\t\t\tprint sunset time\n");
    printf("\t-u, --utc\t\t\tprint times in UTC\n");
    printf("\t-h, --help\t\t\tthis page\n");
    printf("\t--usage\t\t\t\tthis page\n");
    printf("\t-12h\t\t\t\tprint times in 12 hour format\n");
    printf("\t-24h\t\t\t\tprint times in 24 hour format\n");
    printf("\t--seconds\t\t\tprint seconds along with time\n");
}

int main(int argc, char *argv[])
{
    print_conf_t pconf = { .am_pm = true, .seconds = false };
    struct {
        bool silent_mode; /* only print times */
        bool show_future_only; /* only show future times */
        bool reconf; /* reconfigure? */
        bool print_sunset; /* print sunset */
        bool utc; /* use UTC */
        bool help; /* help */
    } conf = { .silent_mode = false,
               .show_future_only = false,
               .reconf = false,
               .print_sunset = false,
               .utc = false,
               .help = false };

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
        argc2++;
    }
    if(conf.help) {
        show_help();
        return 0;
    }
    if(!conf.silent_mode) {
        printf(
            "therealblue24's prayer time calculator\nCopyright (C) 2024 therealblue24 (under MIT license).\n");
        printf("to reconfigure, please run '%s --reconf'\n\n", argv[0]);
    }

    char *home = getenv("HOME");
    if(!home) {
        fprintf(stderr, "failed to obtain $HOME: %s\n", strerror(errno));
        return 1;
    }

    bool configured = false;

    char *dpath = malloc(1024);
    char *rpath = malloc(1024);
    strlcpy(rpath, home, 1024);
    strlcpy(dpath, home, 1024);
    strlcat(dpath, "/.config/prayertimes", 1024);
    strlcat(rpath, "/.config/prayertimes/config.dat", 1024);

    if(access(dpath, F_OK) == -1) {
        if(errno == ENOENT) {
            printf("Creating directory\n");
            if(mkdir(dpath, 0755) == -1) {
                fprintf(stderr, "uh oh! unable to create %s: %s\n", dpath,
                        strerror(errno));
                return 1;
            }
        } else {
            fprintf(stderr, "uh oh! unable to access %s: %s\n", dpath,
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
            fprintf(stderr, "uh oh! unable to access %s: %s\n", rpath,
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
    fread(&lng, sizeof(double), 1, f);
    fread(&lat, sizeof(double), 1, f);
    fread(&ang, sizeof(double), 1, f);
    /* seperated so that it doesn't ruin alignment */
    fread(&elev, sizeof(double), 1, f);

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
    now_suntime += Z;
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
    calc_schedule(lat, lng, elev, Z, now, t, ang);
    double fajr_suntime = t[0];
    double sunrise_suntime = t[1];
    double dhuhr_suntime = t[2];
    double asr_suntime = t[3];
    double sunset_suntime = t[4];
    double maghrib_suntime = t[5];
    double isha_suntime = t[6];

    timelabel fajr = sun2norm(fajr_suntime);
    timelabel sunrise = sun2norm(sunrise_suntime);
    timelabel dhuhr = sun2norm(dhuhr_suntime);
    timelabel asr = sun2norm(asr_suntime);
    timelabel sunset = sun2norm(sunset_suntime);
    timelabel maghrib = sun2norm(maghrib_suntime);
    timelabel isha = sun2norm(isha_suntime);
#define X(s, c, e)                             \
    if(conf.show_future_only && (c) && (e)) {  \
        s;                                     \
    } else if(!conf.show_future_only && (e)) { \
        s;                                     \
    }

    X(print_time("Fajr:    ", fajr, pconf), now_suntime < fajr_suntime, true);
    X(print_time("Sunrise: ", sunrise, pconf), now_suntime < sunrise_suntime,
      true);
    X(print_time("Dhuhr:   ", dhuhr, pconf), now_suntime < dhuhr_suntime, true);
    X(print_time("Asr:     ", asr, pconf), (now_suntime < asr_suntime), true);
    X(print_time("Sunset:  ", sunset, pconf),
      (now_suntime < sunset_suntime) && conf.print_sunset, conf.print_sunset);
    X(print_time("Maghrib: ", maghrib, pconf), now_suntime < maghrib_suntime,
      true);
    X(print_time("Isha:    ", isha, pconf), (now_suntime < isha_suntime), true);
#undef X
}
