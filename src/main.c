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
#include "config.h"

#define strlit(x) strndup(x, strlen(x) + 1)

#define VERSION "2.0-beta3"

#define _str(x)       x
#define xstr(x)       _str(#x)
#define xstrcat(a, b) xstr(a) xstr(b)

struct conf {
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
    double elevation; /* elevation */
    /* latitude & longitude */
    double lat;
    double lng;
    bool shia; /* autodetected */
    bool sunni; /* autodetected */
    bool adjust; /* adjust prayer times */
    int method; /* method */
};

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

void remove_chk(char *path)
{
    if(remove(path) == -1) {
        fprintf(stderr, "ERROR: Failed to delete %s: %s\n", path,
                strerror(errno));
        exit(EXIT_FAILURE);

        printf("Hello.\n");
        return;
    }
    return;
}

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

void init_conf(config_t *cfg, [[maybe_unused]] char *path)
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
        if(ang != 1 && ang != 2) {
            /* the legend of sisyphus */
            goto label0;
        }
    } else {
        printf("Please enter 'Shia' or 'Sunni'.\n");
        exit(EXIT_FAILURE);
    }
    printf("Ok, preparing config file... \n");

#define N(x, y)                                               \
    config_append_val(cfg, (config_val_t){ .name = x,         \
                                           .value_type = NUM, \
                                           .value_num = y,    \
                                           .ignore = false })

    N("latitude", lat);
    N("longitude", lng);
    N("elevation", elev);
    N("asr_shadow_length", ang);

    putchar('\n');

#undef N
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
    printf("\t-e, --elevation\t\t\tset elevation\n");
    printf("\t-fa, --fajr-angle\t\tset fajr angle\n");
    printf("\t-aa, --asr-shadow-length\tset custom asr shadow length\n");
    printf("\t-ia, --isha-angle\t\tset isha angle\n");
    printf("\t-mm, --maghrib-minutes\t\tset maghrib minutes\n");
    printf("\t-ma, --maghrib-angle\t\tset maghrib angle\n");
    printf("\t-im, --imsak-minutes\t\tset imsak minutes\n");
    printf("\t-i,  --isha-minutes\t\tset isha minutes\n");
    printf("\t-d, --default\t\t\tset all settings to default\n");
    printf("\t--ramadan\t\t\tpass this flag if it's Ramadan\n");
    printf("\t--adjust\t\t\tadjust prayer times\n");
    printf("\t-mwl, --mwl-method\t\tuse MWL method\n");
    printf("\t-isna, --isna-method\t\tuse ISNA method\n");
    printf("\t-egypt, --egypt-method\t\tuse Egypt method\n");
    printf("\t-makkah, --makkah-method\tuse Makkah method\n");
    printf("\t-karachi, --karachi-method\tuse Karachi method\n");
    printf("\t-tehran, --tehran-method\tuse Tehran method\n");
    printf("\t-jafari, --jafari-method\tuse Jafari method\n");
    printf("\t--rewrite\t\t\trewrite config\n");
}

void reinterperet_config_values(config_t *cfg)
{
#define X(v, t) config_reinterperet_val(cfg, v, t)
#define B(v)    X(xstr(v), BOOL)
#define N(v)    X(xstr(v), NUM)
#define I(v)    X(xstr(v), INT)
#define S(v)    X(xstr(v), STRING)

    B(silent_mode);
    B(show_future_only);
    B(show_sunset);
    B(utc);
    B(show_imsak);
    B(show_midnight);
    B(adjust);

    N(imsak_minutes);
    N(elevation);
    N(latitude);
    N(longitude);

    N(asr_shadow_length);
    N(fajr_angle);
    N(isha_angle);
    N(maghrib_minutes);
    N(isha_minutes);
    N(maghrib_angle);

    B(use_maghrib_angle);
    B(use_isha_angle);

    B(no_show_fajr);
    B(no_show_sunrise);
    B(no_show_dhuhr);
    B(no_show_asr);
    B(no_show_maghrib);
    B(no_show_isha);

    /* Can't use numbers in front of literals in C! Yay! */
    config_reinterperet_val(cfg, "12_hour_time", BOOL);

    B(show_seconds);
    B(color);
    return;

#undef X
#undef B
#undef N
#undef I
#undef S
}

void load_config_values(config_t *cfg, print_conf_t *pc, struct conf *c)
{
#define B(s, n, v) s = config_getbool(cfg, n, v)
#define N(s, n, v) s = config_getnum(cfg, n, v)
#define I(s, n, v) s = config_getint(cfg, n, v)
#define S(s, n, v) s = config_getstr(cfg, n, v)
#define tc         c->timeconf

    B(c->silent_mode, "silent_mode", false);
    B(c->show_future_only, "show_future_only", false);
    B(c->print_sunset, "show_sunset", false);
    B(c->utc, "utc", false);
    B(c->midnight, "show_midnight", false);
    B(c->imsak, "show_imsak", false);
    B(c->adjust, "adjust", false);

    N(c->imsak_minutes, "imsak_minutes", 15.0);
    N(c->elevation, "elevation", 0.0);
    N(c->lat, "latitude", 0);
    N(c->lng, "longitude", 0);

    N(tc.asr_shadow_length, "asr_shadow_length", 0);
    N(tc.fajr_angle, "fajr_angle", 13.5);
    N(tc.isha_angle, "isha_angle", 14.5);
    N(tc.maghrib_minutes, "maghrib_minutes", 15);
    N(tc.isha_minutes, "isha_minutes", 0);
    N(tc.maghrib_angle, "maghrib_angle", 0);

    B(tc.use_maghrib_angle, "use_maghrib_angle", false);
    B(tc.use_isha_angle, "use_isha_angle", true);

    B(c->no_show[0], "no_show_fajr", false);
    B(c->no_show[1], "no_show_sunrise", false);
    B(c->no_show[2], "no_show_dhuhr", false);
    B(c->no_show[3], "no_show_asr", false);
    B(c->no_show[5], "no_show_maghrib", false);
    B(c->no_show[6], "no_show_isha", false);

    B(pc->seconds, "show_seconds", false);
    B(pc->am_pm, "12_hour_time", true);
    B(pc->color, "color", false);

    char *method = config_getstr(cfg, "method", "none");
#define C(s, n)                        \
    if(strcmp(method, xstr(s)) == 0) { \
        c->method = n;                 \
    }
    C(none, 0);
    C(mwl, 1);
    C(isna, 2);
    C(egypt, 3);
    C(makkah, 4);
    C(karachi, 5);
    C(tehran, 6);
    C(jafari, 7);

#undef B
#undef N
#undef I
#undef S
#undef tc
}

static const struct conf default_conf = {
    .silent_mode = false,
    .show_future_only = false,
    .reconf = false,
    .print_sunset = false,
    .utc = false,
    .help = false,
    .midnight = false,
    .imsak = false,
    .imsak_minutes = 15.0,
    .timeconf = { .asr_shadow_length = 0,
                 .fajr_angle = 13.5,
                 .isha_angle = 14.5,
                 .maghrib_minutes = 15,
                 .isha_minutes = 0,
                 .maghrib_angle = 0,
                 .use_maghrib_angle = false,
                 .use_isha_angle = true },
    .shia = false,
    .sunni = false,
    .adjust = false,
    .elevation = 0,
    .method = 0,
};

int main(int argc, char *argv[])
{
    bool midnight_using_fajr = false;
    bool emit_file = false;
    config_t *cfg = config_init();
    assert(cfg);

    static const char *methods[] = { "none",   "mwl",     "isna",   "egypt",
                                     "makkah", "karachi", "tehran", "jafari" };

    int ramadan = 0;
    print_conf_t pconf = { .am_pm = true, .seconds = false, .color = false };
    struct conf conf = default_conf;
    bool set_to_default = false;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--default") == 0) {
            set_to_default = true;
        }
    }

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--ramadan") == 0) {
            ramadan = 1;
        }
        if(strcmp(argv[i], "--reconf") == 0 || strcmp(argv[i], "-rc") == 0) {
            conf.reconf = true;
        }
        if(strcmp(argv[i], "--rewrite") == 0) {
            emit_file = true;
        }
    }

    if(conf.reconf) {
        emit_file = true;
    }

    char *home = getenv("HOME");
    if(!home) {
        fprintf(stderr, "ERROR: failed to obtain $HOME: %s\n", strerror(errno));
        return 1;
    }

    bool configured = false;

    char *dpath = calloc(1, 1024);
    char *rpath = calloc(1, 1024);
    char *cpath = calloc(1, 1024);
    if(!dpath || !rpath || !cpath) {
        fprintf(stderr, "ERROR: Failed to allocate memory. ENOMEM?: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    strlcpy(rpath, home, 1024);
    strlcpy(dpath, home, 1024);
    strlcpy(cpath, home, 1024);
    strlcat(dpath, "/.config/prayertimes", 1024);
    strlcat(rpath, "/.config/prayertimes/config.dat", 1024);
    strlcat(cpath, "/.config/prayertimes/config.yml", 1024);

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

    bool old_config_not_found = access(rpath, F_OK) == -1;

    if(old_config_not_found && errno != ENOENT) {
        fprintf(stderr, "ERROR: unable to access %s: %s\n", rpath,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    bool new_config_not_found = access(cpath, F_OK) == -1;

    if(new_config_not_found && errno != ENOENT) {
        fprintf(stderr, "ERROR: unable to access %s: %s\n", cpath,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(!configured && conf.reconf) {
        init_conf(cfg, rpath);
        configured = true;
    }

    if(new_config_not_found && !configured && old_config_not_found) {
        printf("Configuring for first use\n");
        init_conf(cfg, rpath);
        configured = true;
    }

    if(!old_config_not_found) {
        double lat, lng, elev, ang = 0;
        printf("Migrating config... ");
        FILE *f = fopen(rpath, "rb");
        if(!f) {
            fprintf(stderr, "\nERROR: Failed to open configuration file: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        assert(fread(&lng, sizeof(double), 1, f));
        assert(fread(&lat, sizeof(double), 1, f));
        assert(fread(&ang, sizeof(double), 1, f));
        /* seperated so that it doesn't ruin alignment */
        assert(fread(&elev, sizeof(double), 1, f));
        fclose(f);

#define N(x, y)                                               \
    config_append_val(cfg, (config_val_t){ .name = x,         \
                                           .value_type = NUM, \
                                           .value_num = y,    \
                                           .ignore = false })

        N("latitude", lat);
        N("longitude", lng);
        N("elevation", elev);
        N("asr_shadow_length", ang);

        conf.timeconf.asr_shadow_length = ang;
        conf.elevation = elev;
        conf.lat = lat;
        conf.lng = lng;

        emit_file = true;

        remove_chk(rpath);
        old_config_not_found = true;
        printf("done\n");
#undef N
    }

    time_t now = time(NULL);
    double now_suntime = suntime_now(now);

    if(!set_to_default) {
        assert(config_load(cfg, cpath) == 0);
        reinterperet_config_values(cfg);
        load_config_values(cfg, &pconf, &conf);
    }

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
        if(strcmp(arg, "-aa") == 0 || strcmp(arg, "--asr-shadow-length") == 0) {
            double asr_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.asr_shadow_length = asr_angle;
        }
        if(strcmp(arg, "-ia") == 0 || strcmp(arg, "--isha-angle") == 0) {
            double isha_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.isha_angle = isha_angle;
        }
        if(strcmp(arg, "-mm") == 0 || strcmp(arg, "--maghrib-minutes") == 0) {
            double maghrib_minutes = strtod(argv[++argc2], NULL);
            conf.timeconf.maghrib_minutes = maghrib_minutes;
        }
        if(strcmp(arg, "-i") == 0 || strcmp(arg, "--isha-minutes") == 0) {
            double isha_minutes = strtod(argv[++argc2], NULL);
            conf.timeconf.isha_minutes = isha_minutes;
        }
        if(strcmp(arg, "-ma") == 0 || strcmp(arg, "--maghrib-angle") == 0) {
            double maghrib_angle = strtod(argv[++argc2], NULL);
            conf.timeconf.maghrib_angle = maghrib_angle;
            conf.timeconf.use_maghrib_angle = true;
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
        if(strcmp(arg, "-e") == 0 || strcmp(arg, "--elevation") == 0) {
            double elev = strtod(argv[++argc2], NULL);
            conf.elevation = elev;
        }

        /* clang-format off */
#define methodchk(name,n)                      \
    if(strcmp(arg, xstrcat(-, name)) == 0 || \
       strcmp(arg, xstrcat(--, name-method)) == 0 || conf.method == n)
        /* clang-format on */
#define tc conf.timeconf
#define c  conf

        /* Methods refrenced from this page: http://praytimes.org/wiki/Calculation_Methods */

        methodchk(mwl, 1)
        {
            tc.fajr_angle = 18;
            tc.isha_angle = 17;
            tc.maghrib_minutes = 0;
            c.method = 1;
        }

        methodchk(isna, 2)
        {
            tc.fajr_angle = 15;
            tc.isha_angle = 15;
            tc.maghrib_minutes = 0;
            c.method = 2;
        }

        methodchk(egypt, 3)
        {
            tc.fajr_angle = 19.5;
            tc.isha_angle = 17.5;
            tc.maghrib_minutes = 0;
            c.method = 3;
        }

        methodchk(makkah, 4)
        {
            tc.fajr_angle = 18.5;
            tc.use_isha_angle = false;
            tc.isha_minutes = 90 + (30 * ramadan);
            c.method = 4;
        }

        methodchk(karachi, 5)
        {
            tc.fajr_angle = 18;
            tc.isha_angle = 18;
            tc.maghrib_minutes = 0;
            c.method = 5;
        }

        methodchk(tehran, 6)
        {
            tc.fajr_angle = 17.7;
            tc.isha_angle = 14;
            tc.use_maghrib_angle = true;
            tc.maghrib_angle = 4.5;
            tc.maghrib_minutes = 0;
            midnight_using_fajr = true;
            c.method = 6;
        }

        methodchk(jafari, 7)
        {
            tc.fajr_angle = 16;
            tc.isha_angle = 14;
            tc.use_maghrib_angle = true;
            tc.maghrib_angle = 4;
            tc.maghrib_minutes = 0;
            midnight_using_fajr = true;
            c.method = 7;
        }

#undef methodchk
#undef tc
#undef c
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

#define methodchk(x, y) if(y == conf.method)
#define c               conf
#define tc              conf.timeconf

    methodchk(mwl, 1)
    {
        tc.fajr_angle = 18;
        tc.isha_angle = 17;
        tc.maghrib_minutes = 0;
        c.method = 1;
    }

    methodchk(isna, 2)
    {
        tc.fajr_angle = 15;
        tc.isha_angle = 15;
        tc.maghrib_minutes = 0;
        c.method = 2;
    }

    methodchk(egypt, 3)
    {
        tc.fajr_angle = 19.5;
        tc.isha_angle = 17.5;
        tc.maghrib_minutes = 0;
        c.method = 3;
    }

    methodchk(makkah, 4)
    {
        tc.fajr_angle = 18.5;
        tc.use_isha_angle = false;
        tc.isha_minutes = 90 + (30 * ramadan);
        c.method = 4;
    }

    methodchk(karachi, 5)
    {
        tc.fajr_angle = 18;
        tc.isha_angle = 18;
        tc.maghrib_minutes = 0;
        c.method = 5;
    }

    methodchk(tehran, 6)
    {
        tc.fajr_angle = 17.7;
        tc.isha_angle = 14;
        tc.use_maghrib_angle = true;
        tc.maghrib_angle = 4.5;
        tc.maghrib_minutes = 0;
        midnight_using_fajr = true;
        c.method = 6;
    }

    methodchk(jafari, 7)
    {
        tc.fajr_angle = 16;
        tc.isha_angle = 14;
        tc.use_maghrib_angle = true;
        tc.maghrib_angle = 4;
        tc.maghrib_minutes = 0;
        midnight_using_fajr = true;
        c.method = 7;
    }
#undef methodchk
#undef c
#undef tc

    const double shia_asr_shadow_length = (double)2 / (double)7;

    /* Simple shia/sunni detection */
    if(conf.timeconf.asr_shadow_length == shia_asr_shadow_length) {
        conf.shia = true;
        midnight_using_fajr = true;
    } else {
        conf.sunni = true;
    }

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

    calc_schedule(conf.lat, conf.lng, conf.elevation, Z, now, t, conf.timeconf);
    if(conf.adjust) {
        adjust_times(conf.lat, conf.lng, conf.elevation, Z, now, t,
                     conf.timeconf);
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

    if(conf.shia || midnight_using_fajr) {
        double span = bound_hour(fajr_suntime - sunset_suntime);
        midnight_suntime = sunset_suntime + (span / 2);
    } else if(conf.sunni || !midnight_using_fajr) {
        double span = bound_hour(sunrise_suntime - sunset_suntime);
        midnight_suntime = sunset_suntime + (span / 2);
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
    if(emit_file) {
#define c    conf
#define tc   conf.timeconf
#define S(x) (conf.no_show[x])

        config_append_bool(cfg, strlit("silent_mode"), c.silent_mode);
        config_append_bool(cfg, strlit("show_future_only"), c.show_future_only);
        config_append_bool(cfg, strlit("show_sunset"), c.print_sunset);
        config_append_bool(cfg, strlit("utc"), c.utc);
        config_append_bool(cfg, strlit("show_imsak"), c.imsak);
        config_append_bool(cfg, strlit("show_midnight"), c.midnight);
        config_append_bool(cfg, strlit("adjust"), c.adjust);

        config_append_num(cfg, strlit("imsak_minutes"), c.imsak_minutes);
        config_append_num(cfg, strlit("elevation"), c.elevation);
        config_append_num(cfg, strlit("latitude"), c.lat);
        config_append_num(cfg, strlit("longitude"), c.lng);

        config_append_num(cfg, strlit("asr_shadow_length"),
                          tc.asr_shadow_length);
        config_append_num(cfg, strlit("fajr_angle"), tc.fajr_angle);
        config_append_num(cfg, strlit("isha_angle"), tc.isha_angle);
        config_append_num(cfg, strlit("maghrib_minutes"), tc.maghrib_minutes);
        config_append_num(cfg, strlit("isha_minutes"), tc.isha_minutes);
        config_append_num(cfg, strlit("maghrib_angle"), tc.maghrib_angle);

        config_append_bool(cfg, strlit("use_maghrib_angle"),
                           tc.use_maghrib_angle);
        config_append_bool(cfg, strlit("use_isha_angle"), tc.use_isha_angle);

        config_append_bool(cfg, strlit("no_show_fajr"), S(0));
        config_append_bool(cfg, strlit("no_show_sunrise"), S(1));
        config_append_bool(cfg, strlit("no_show_dhuhr"), S(2));
        config_append_bool(cfg, strlit("no_show_asr"), S(3));
        config_append_bool(cfg, strlit("no_show_maghrib"), S(5));
        config_append_bool(cfg, strlit("no_show_isha"), S(6));

        config_append_str(cfg, strlit("method"), strdup(methods[c.method]));

        config_append_bool(cfg, strlit("12_hour_time"), pconf.am_pm);
        config_append_bool(cfg, strlit("show_seconds"), pconf.seconds);
        config_append_bool(cfg, strlit("color"), pconf.color);

        FILE *emit_to = fopen(cpath, "w");
        assert(emit_to);

        config_emit(cfg, emit_to);
        fclose(emit_to);
    }
#undef X
#undef c
#undef tc
#undef S

    config_free(cfg);
    return 0;
}
