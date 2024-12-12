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

int main(int argc, char *argv[])
{
    printf(
        "therealblue24's prayer time calculator\nCopyright (C) 2024 therealblue24 (under MIT license).\n");
    printf("to reconfigure, please run '%s --reconf'\n\n", argv[0]);

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
    if(argc > 1) {
        if(strcmp(argv[1], "--reconf") == 0) {
            if(!configured)
                init_conf(rpath);
            configured = true;
        }
    }

    print_conf_t pconf = { .am_pm = true, .seconds = false };

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
        argc2++;
    }

    double lat, lng, elev, ang = 0;
    time_t now = time(NULL);

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
    double Z = (double)tm.tm_gmtoff / (60 * 60);
    printf("Time zone: %g hours (%s)\n", Z, tm.tm_zone);

    printf("Prayer times for %s, %s %d%s, %d:\n", weekdays[tm.tm_wday],
           months[tm.tm_mon], tm.tm_mday, prefixes[tm.tm_mday],
           tm.tm_year + 1900);
    double t[7] = { 0 };
    calc_schedule(lat, lng, elev, Z, now, t, ang);
    //double bedtime_ = ((t[5] + t[6]) / 2.) + 1;
    timelabel fajr = sun2norm(t[0]);
    timelabel sunrise = sun2norm(t[1]);
    timelabel dhuhr = sun2norm(t[2]);
    timelabel asr = sun2norm(t[3]);
    //timelabel sunset = sun2norm(t[4]);
    timelabel maghrib = sun2norm(t[5]);
    timelabel isha = sun2norm(t[6]);
    //timelabel bedtime = sun2norm(bedtime_);

    //printf("WARNING: DO NOT 100%% TRUST THESE TIMES.\n");
    //printf("WARNING: Asr may not be accurate.\n");
    print_time("Fajr:    ", fajr, pconf);
    print_time("Sunrise: ", sunrise, pconf);
    print_time("Dhuhr:   ", dhuhr, pconf);
    print_time("Asr:     ", asr, pconf);
    //print_time("Sunset:  ", sunset); useless in most cases
    print_time("Maghrib: ", maghrib, pconf);
    print_time("Isha:    ", isha, pconf);
    //putchar('\n');
    //print_time("Bedtime: ", bedtime);
}
