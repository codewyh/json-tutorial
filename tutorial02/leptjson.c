#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static int validateNumber(const char* num);
static int validateExp(const char* num);
static int validateFrac(const char* num);
static int validateInt(const char* num);


static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, char* word, lept_type type) {
    char ch = *word++;
    int i = 0;
    EXPECT(c, ch);
   
    ch = *word++;
    while (ch != '\0') {
        if (c->json[i] != ch) 
            return LEPT_PARSE_INVALID_VALUE;
        ++i;
        ch = *word++;
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    
    int ret = validateNumber(c->json);
    if (ret != 0) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int validateExp(const char* num) {
    char c;
    if (num == NULL || *num == '\0')
        return 1;

    c = *num;
    if (c == 'e' || c == 'E') {
        c = *(++num);
        if (c == '+' || c == '-') {
            c = *(++num);
        }
        while (c != '\0') {
           if (!ISDIGIT(c)) {
               return 2;
           } else {
               c = *(++num);
           }
        }
    } else {
        return 2;
    }
    return 0;
}

static int validateFrac(const char* num) {
    char c;
    if (num == NULL || *num == '\0')
        return 1;

    c = *num;
    if (c == '.') {
        c = *(++num);
        while (c != '\0') {
            if (ISDIGIT(c)) {
                c = *(++num);
            } else if (c == 'e' || c == 'E') {
                return validateExp(num);
            } else {
                return 2;
            }
        }
    } else {
        return 3;
    }
    return 0;
}

static int validateInt(const char* num) {
    char c;
    if (num == NULL || *num == '\0')
        return 1;

    c = *num;
    if (c == '0') {
        return validateFrac(++num);
    } else if (ISDIGIT1TO9(c)) {
        c = *(++num);
        while (c != '\0') {
            if (ISDIGIT(c)) {
                c = *(++num);
            } else if (c == '.') {
                return validateFrac(num);
            } else if (c == 'e' || c == 'E') {
                return validateExp(num);
            } else {
                return 2;
            }
        }
        return 0;
    }
    return 2;
}

static int validateNumber(const char* num) {
    char c;
    if (num == NULL || *num == '\0')
        return 1;

    c = *num;
    if (c == '-') {
        ++num;
    }
    return validateInt(num);
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
