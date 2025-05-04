#include "config.h"
#include <stdatomic.h>
#include <stdlib.h>

static int iswhitespace(char c)
{
    return c == ' ' || c == '\t';
}

/* Free a config value */
void config_val_free(config_val_t val)
{
    /* Shhhh.... it's my top debug strategy. */
    /* printf("free: %s %p\n", val.name, val.val.str); */
    free(val.name);
    if(val.value_type == STRING) {
        free(val.val.str);
    }
    return;
}

/* Free a config */
void config_free(config_t *cfg)
{
    for(size_t i = 0; i < cfg->size; i++) {
        config_val_free(cfg->vals[i]);
    }
    free(cfg->vals);
    free(cfg);
}

/* Get a value's location given name */
optsize_t config_get_val_loc(config_t *cfg, char *name)
{
    for(size_t i = 0; i < cfg->size; i++) {
        if(strcmp(cfg->vals[i].name, name) == 0 && !cfg->vals[i].notexist) {
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

    bool found;
    found = false;
    size_t l;

    optsize_t l_ = config_get_val_loc(cfg, val.name);
    if(l_.has_value) {
        found = true;
        l = l_.val;
        goto out;
    }
    /* Try to find an unused slot */

    for(l = 0; l < cfg->size; l++) {
        if(cfg->vals[l].notexist) {
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

/* Next few functions are basically boilerplate helpers */

int config_append_str(config_t *cfg, char *name, char *val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = STRING,
                                                  .val.str = val });
}

int config_append_num(config_t *cfg, char *name, double val)
{
    return config_append_val(
        cfg, (config_val_t){ .name = name, .value_type = NUM, .val.num = val });
}

int config_append_int(config_t *cfg, char *name, int val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = INT,
                                                  .val.integer = val });
}

int config_append_bool(config_t *cfg, char *name, bool val)
{
    return config_append_val(cfg, (config_val_t){ .name = name,
                                                  .value_type = BOOL,
                                                  .val.boolean = val });
}

/* Delete a value given name */
int config_delete_val(config_t *cfg, char *name)
{
    optsize_t loc = config_get_val_loc(cfg, name);
    if(!loc.has_value) {
        return 1;
    }
    cfg->vals[loc.val].notexist = true;
    return 0;
}

/* Reinterperet a value from string -> ... */
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

    char *str;
    str = strndup(val.val.str, 512);
    if(newtype != STRING) {
        free(val.val.str);
    }
    val.value_type = newtype;
    switch(newtype) {
    case STRING:
        break;
    case NUM:
        val.val.num = strtod(str, NULL);
        break;
    case INT:
        val.val.integer = (int)strtol(str, NULL, 10);
        break;
    case BOOL:;
        int istrue, isfalse;
        istrue = strcmp(str, "true") == 0;
        isfalse = strcmp(str, "false") == 0;
        if(istrue) {
            val.val.boolean = true;
        }
        if(isfalse) {
            val.val.boolean = false;
        }
        break;
    default:
        __builtin_unreachable();
    }
    free(str);
    return config_set_val(cfg, name, val);
}

/* Emit a config in mem to a file */
int config_emit(config_t *cfg, FILE *out)
{
    static const char *bool_to_string[] = { "false", "true", NULL };
    for(size_t i = 0; i < cfg->size; i++) {
        config_val_t v = cfg->vals[i];
        fprintf(out, "%s: ", v.name);
        switch(v.value_type) {
        case STRING:
            fprintf(out, "%s\n", v.val.str);
            break;
        case NUM:
            fprintf(out, "%lf\n", v.val.num);
            break;
        case INT:
            fprintf(out, "%d\n", v.val.integer);
            break;
        case BOOL:
            fprintf(out, "%s\n", bool_to_string[v.val.boolean]);
            break;
        default:
            return 1;
        }
    }
    return 0;
}

/* debug */
__attribute__((unused)) static void config_print_val(config_val_t v)
{
    static const char *bool_to_string[] = { "false", "true", NULL };
    printf("%s: ", v.name);
    switch(v.value_type) {
    case STRING:
        printf("%s\n", v.val.str);
        return;
    case NUM:
        printf("%lf\n", v.val.num);
        return;
    case INT:
        printf("%d\n", v.val.integer);
        return;
    case BOOL:
        printf("%s\n", bool_to_string[v.val.boolean]);
        return;
    default:
        __builtin_unreachable();
    }
}

/* get a value from the config */
config_val_t config_get_val(config_t *cfg, char *name)
{
    const config_val_t null_value = (config_val_t){
        .name = name, .notexist = true, .value_type = STRING, .val.str = NULL
    };

    optsize_t loc = config_get_val_loc(cfg, name);
    if(loc.has_value) {
        return cfg->vals[loc.val];
    }
    return null_value;
}

/* set a value to the config */
int config_set_val(config_t *cfg, char *name, config_val_t val)
{
    optsize_t loc = config_get_val_loc(cfg, name);
    if(!loc.has_value) {
        return 1;
    }
    cfg->vals[loc.val] = val;
    return 0;
}

/* allocate a config */
config_t *config_init(void)
{
    config_t *cfg = calloc(1, sizeof(config_t));
    return cfg;
}

/* load a config given file name */
int config_load(config_t *cfg, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if(!f) {
        return 1;
    }

    size_t size;
    size = 0;
    fseek(f, 0, SEEK_END);
    size = (size_t)ftell(f);
    rewind(f);

    char name[100] = { 0 };
    size_t name_l = 0;
    config_val_t v = { .value_type = STRING, .val.str = NULL };
    size_t i = 0;
    bool did_something = false;
    while(i < size) {
        did_something = false;
        char c;
        c = (char)fgetc(f);
        if(iswhitespace(c) || c == '\r' || c == '\n') {
            continue;
        }
        if(c == '#') {
            printf("please don't use comments in the config it screws "
                   "everything up\n");
            printf("solution coming soon (TM)\n");
            exit(EXIT_FAILURE);
            /*
            while(i < size && c != '\n') {
                c = fgetc(f);
                i++;
            }
            continue;
*/
        }

        if(c == ':') {
            c = (char)fgetc(f);
            i++;
            while(i < size && iswhitespace(c)) {
                c = (char)fgetc(f);
                i++;
            }
            size_t j;
            j = i;
            while(j < size && c != '\n') {
                c = (char)fgetc(f);
                j++;
            }
            fseek(f, (long)i, SEEK_SET);
            char *str = calloc(1, (j - i) + 1);
            fread(str, (j - i), 1, f);
            for(size_t k = 0; k < 100; k++) {
                if(name[k] == '\n' || name[k] == '\r' ||
                   iswhitespace(name[k])) {
                    name[k] = '\0';
                }
            }
            for(size_t k = 0; k < (j - i) + 1; k++) {
                if(str[k] == '\n' || str[k] == '\r') {
                    str[k] = '\0';
                }
            }
            i = j;
            do {
                c = (char)fgetc(f);
                i++;
            } while(i < size && c != '\n');
            v.name = strndup(name, 100);
            v.val.str = strndup(str, 512);
            v.notexist = false;
            v.value_type = STRING;
            config_append_val(cfg, v);
            name_l = 0;
            memset(name, 0, 100);
            did_something = true;
            free(str);
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

/* alloc + load */
config_t *config_from(const char *filename)
{
    config_t *cfg = config_init();
    config_load(cfg, filename);
    return cfg;
}

/* boilerplate */

char *config_getstr(config_t *cfg, char *name, char *default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.val.str;
}

double config_getnum(config_t *cfg, char *name, double default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.val.num;
}

int config_getint(config_t *cfg, char *name, int default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.val.integer;
}

bool config_getbool(config_t *cfg, char *name, bool default_)
{
    config_val_t v = config_get_val(cfg, name);
    if(v.notexist) {
        return default_;
    }
    return v.val.boolean;
}
