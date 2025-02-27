#include "config.h"
#include <stdatomic.h>
#include <stdlib.h>

int iswhitespace(char c)
{
    return c == ' ' || c == '\t';
}

void config_val_free(config_val_t val)
{
    free(val.name);
    if(val.value_type == STRING) {
        free(val.value_str);
    }
    free(val.comment);
    return;
}

void config_free(config_t *cfg)
{
    for(size_t i = 0; i < cfg->size; i++) {
        config_val_free(cfg->vals[i]);
    }
    free(cfg->vals);
    free(cfg);
}

optsize_t config_get_val_loc(config_t *cfg, char *name)
{
    for(size_t i = 0; i < cfg->size; i++) {
        if(strcmp(cfg->vals[i].name, name) == 0 && !cfg->vals[i].ignore) {
            return (optsize_t){ .has_value = true, .val = i };
        }
    }
    return (optsize_t){ .has_value = false };
}

/* Append a value to the config */
int config_append_val(config_t *cfg, config_val_t val)
{
    if(!cfg) {
        return 1;
    }

    /* Initalize if neccessary */
    if(cfg->cap == 0) {
        cfg->cap = 100;
        cfg->size = 0;
        cfg->vals = calloc(cfg->cap, sizeof(config_val_t));
    }

    if(!cfg->vals) {
        return 1;
    }

    bool found = false;
    size_t l;

    optsize_t l_ = config_get_val_loc(cfg, val.name);
    if(l_.has_value) {
        found = true;
        l = l_.val;
        goto out;
    }
    /* Try to find an unused slot */

    for(l = 0; l < cfg->size; l++) {
        if(cfg->vals[l].ignore) {
            found = true;
            goto out;
        }
    }
    goto out;
out:;

    if(found) {
        /* Just use that slot */
        cfg->vals[l] = val;
        return 0;
    }

    /* Do we need to resize? */
    if(cfg->size >= cfg->cap) {
        /* Resize. */
        cfg->cap += 100;
        cfg->vals = realloc(cfg->vals, sizeof(config_val_t) * cfg->cap);
        if(!cfg->vals) {
            return 1;
        }
    }

    /* Append */
    cfg->vals[cfg->size++] = val;

    return 0;
}

int config_append_str(config_t *cfg, char *name, char *val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = STRING,
                                                  .value_str = val });
}

int config_append_num(config_t *cfg, char *name, double val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = NUM,
                                                  .value_num = val });
}

int config_append_int(config_t *cfg, char *name, int val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = INT,
                                                  .value_int = val });
}

int config_append_bool(config_t *cfg, char *name, bool val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = BOOL,
                                                  .value_bool = val });
}

int config_delete_val(config_t *cfg, char *name)
{
    optsize_t loc = config_get_val_loc(cfg, name);
    if(!loc.has_value) {
        return 1;
    }
    cfg->vals[loc.val].ignore = true;
    return 0;
}

int config_reinterperet_val(config_t *cfg, char *name, int newtype)
{
    config_val_t val = config_get_val(cfg, name);
    if(val.notexist) {
        return 1;
    }
    if(val.value_type == newtype) {
        return 0;
    }

    if(val.value_type != STRING) {
        fprintf(stderr, "TODO: Non-string value reinterperation\n");
        exit(EXIT_FAILURE);
    }

    char *str = strndup(val.value_str, 512);
    if(newtype != STRING) {
        free(val.value_str);
    }
    val.value_type = newtype;
    switch(newtype) {
    case STRING:
        break;
    case NUM:
        val.value_num = strtod(str, NULL);
        break;
    case INT:
        val.value_int = (int)strtol(str, NULL, 10);
        break;
    case BOOL:;
        int _true = strcmp(str, "true") == 0;
        int _false = strcmp(str, "false") == 0;
        if(_true) {
            val.value_bool = true;
        }
        if(_false) {
            val.value_bool = false;
        }
        break;
    }
    free(str);
    return config_set_val(cfg, name, val);
}

int config_emit(config_t *cfg, FILE *out)
{
    static const char *bool_to_string[] = { "false", "true", NULL };
    for(size_t i = 0; i < cfg->size; i++) {
        config_val_t v = cfg->vals[i];
        if(v.comment) {
            fprintf(out, "# %s\n", v.comment);
        }
        fprintf(out, "%s: ", v.name);
        switch(v.value_type) {
        case STRING:
            fprintf(out, "%s\n", v.value_str);
            break;
        case NUM:
            fprintf(out, "%lf\n", v.value_num);
            break;
        case INT:
            fprintf(out, "%d\n", v.value_int);
            break;
        case BOOL:
            fprintf(out, "%s\n", bool_to_string[v.value_bool]);
            break;
        default:
            return 1;
        }
    }
    return 0;
}

[[maybe_unused]] static void config_print_val(config_val_t v)
{
    static const char *bool_to_string[] = { "false", "true", NULL };
    printf("%s: ", v.name);
    switch(v.value_type) {
    case STRING:
        printf("%s\n", v.value_str);
        return;
    case NUM:
        printf("%lf\n", v.value_num);
        return;
    case INT:
        printf("%d\n", v.value_int);
        return;
    case BOOL:
        printf("%s\n", bool_to_string[v.value_bool]);
        return;
    default:
        return;
    }
    return;
}

config_val_t config_get_val(config_t *cfg, char *name)
{
    const config_val_t null_value = (config_val_t){
        .name = name, .notexist = true, .value_type = STRING, .value_str = NULL
    };

    optsize_t loc = config_get_val_loc(cfg, name);
    if(loc.has_value) {
        return cfg->vals[loc.val];
    }
    return null_value;
}

int config_set_val(config_t *cfg, char *name, config_val_t val)
{
    optsize_t loc = config_get_val_loc(cfg, name);
    if(!loc.has_value) {
        return 1;
    }
    cfg->vals[loc.val] = val;
    return 0;
}

config_t *config_init(void)
{
    config_t *cfg = calloc(1, sizeof(config_t));
    return cfg;
}

int config_load(config_t *cfg, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if(!f) {
        return 1;
    }

    size_t size = 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    char name[100] = { 0 };
    size_t name_l = 0;
    config_val_t v = { .value_type = STRING, .value_str = NULL };
    size_t i = 0;
    bool did_something = false;
    while(i < size) {
        did_something = false;
        char c = fgetc(f);
        if(iswhitespace(c) || c == '\r' || c == '\n') {
            continue;
        }
        if(c == '#') {
            while(i < size && c != '\n') {
                c = fgetc(f);
                i++;
            }
            continue;
        }

        if(c == ':') {
            c = fgetc(f);
            i++;
            while(i < size && iswhitespace(c)) {
                c = fgetc(f);
                i++;
            }
            size_t j = i;
            while(j < size && c != '\n') {
                c = fgetc(f);
                j++;
            }
            fseek(f, i, SEEK_SET);
            char *str = calloc(1, (j - i) + 1);
            fread(str, (j - i), 1, f);
            for(size_t i = 0; i < 100; i++) {
                if(name[i] == '\n' || name[i] == '\r' ||
                   iswhitespace(name[i])) {
                    name[i] = '\0';
                }
            }
            for(size_t i = 0; i < (j - i) + 1; i++) {
                if(str[i] == '\n' || str[i] == '\r') {
                    str[i] = '\0';
                }
            }
            i = j;
            do {
                c = fgetc(f);
                i++;
            } while(i < size && c != '\n');
            v.name = strndup(name, 100);
            v.value_str = str;
            v.ignore = false;
            v.value_type = STRING;
            config_append_val(cfg, v);
            name_l = 0;
            memset(name, 0, 100);
            did_something = true;
            continue;
        }

        if(!did_something) {
            name[name_l++] = c;
        }

        i++;
    }

    fclose(f);
    return 0;
}

config_t *config_from(const char *filename)
{
    config_t *cfg = config_init();
    config_load(cfg, filename);
    return cfg;
}

char *config_getstr(config_t *cfg, char *name, char *default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.value_str;
}

double config_getnum(config_t *cfg, char *name, double default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.value_num;
}

int config_getint(config_t *cfg, char *name, int default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.value_int;
}

bool config_getbool(config_t *cfg, char *name, bool default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.value_bool;
}

void config_set_comment(config_t *cfg, char *name, char *comment)
{
    optsize_t loc = config_get_val_loc(cfg, name);
    if(!loc.has_value) {
        return;
    }

    config_val_t v = cfg->vals[loc.val];
    if(v.comment) {
        free(v.comment);
    }
    v.comment = comment;

    cfg->vals[loc.val] = v;
    return;
}
