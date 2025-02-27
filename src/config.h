#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

/* A config value */
typedef struct config_val {
    char *comment; /* For emitting a value */
    char *name;
    enum { STRING = 0, NUM, INT, BOOL } value_type;
    union {
        char *value_str;
        double value_num;
        int value_int;
        bool value_bool;
    };
    union {
        bool ignore;
        bool notexist;
    }; /* Has a double meaning: either ignore this or this value doesn't exist */
} config_val_t;

/* A config */
typedef struct config {
    config_val_t *vals;
    size_t size;
    size_t cap;
} config_t;

typedef struct optsize {
    bool has_value;
    size_t val;
} optsize_t;

config_t *config_from(const char *file_name);
config_t *config_init(void);
int config_load(config_t *cfg, const char *file_name);
void config_free(config_t *cfg);
void config_val_free(config_val_t val);

int config_append_val(config_t *cfg, config_val_t val);

int config_append_str(config_t *cfg, char *name, char *val);
int config_append_num(config_t *cfg, char *name, double val);
int config_append_int(config_t *cfg, char *name, int val);

int config_append_bool(config_t *cfg, char *name, bool val);

int config_delete_val(config_t *cfg, char *name);
int config_emit(config_t *cfg, FILE *out);

void config_set_comment(config_t *cfg, char *name, char *comment);

optsize_t config_get_val_loc(config_t *cfg, char *name);
config_val_t config_get_val(config_t *cfg, char *name);

char *config_getstr(config_t *cfg, char *name, char *default_);
double config_getnum(config_t *cfg, char *name, double default_);
int config_getint(config_t *cfg, char *name, int default_);
bool config_getbool(config_t *cfg, char *name, bool default_);

int config_set_val(config_t *cfg, char *name, config_val_t val);
int config_reinterperet_val(config_t *cfg, char *name, int newtype);

#endif /* CONFIG_H_ */
