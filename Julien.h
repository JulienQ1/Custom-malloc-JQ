#ifndef Julien_H_
#define Julien_H_

#ifndef Julien_SCOPES_CAPACITY
#define Julien_SCOPES_CAPACITY 128
#endif // Julien_SCOPES_CAPACITY

typedef void* Julien_Sink;
typedef size_t (*Julien_Write)(const void *ptr, size_t size, size_t nmemb, Julien_Sink sink);

typedef enum {
    Julien_OK = 0,
    Julien_WRITE_ERROR,
    Julien_SCOPES_OVERFLOW,
    Julien_SCOPES_UNDERFLOW,
    Julien_OUT_OF_SCOPE_KEY,
    Julien_DOUBLE_KEY
} Julien_Error;

const char *Julien_error_string(Julien_Error error);

typedef enum {
    Julien_ARRAY_SCOPE,
    Julien_OBJECT_SCOPE,
} Julien_Scope_Kind;

typedef struct {
    Julien_Scope_Kind kind;
    int tail;
    int key;
} Julien_Scope;

typedef struct {
    Julien_Sink sink;
    Julien_Write write;
    Julien_Error error;
    Julien_Scope scopes[Julien_SCOPES_CAPACITY];
    size_t scopes_size;
} Julien;

void Julien_null(Julien *Julien);
void Julien_bool(Julien *Julien, int boolean);
void Julien_integer(Julien *Julien, long long int x);
void Julien_float(Julien *Julien, double x, int precision);
void Julien_string(Julien *Julien, const char *str);
void Julien_string_sized(Julien *Julien, const char *str, size_t size);

void Julien_element_begin(Julien *Julien);
void Julien_element_end(Julien *Julien);

void Julien_array_begin(Julien *Julien);
void Julien_array_end(Julien *Julien);

void Julien_object_begin(Julien *Julien);
void Julien_member_key(Julien *Julien, const char *str);
void Julien_member_key_sized(Julien *Julien, const char *str, size_t size);
void Julien_object_end(Julien *Julien);

#endif // Julien_H_

#ifdef Julien_IMPLEMENTATION

static size_t Julien_strlen(const char *s)
{
    size_t count = 0;
    while (*(s + count)) {
        count += 1;
    }
    return count;
}

static void Julien_scope_push(Julien *Julien, Julien_Scope_Kind kind)
{
    if (Julien->error == Julien_OK) {
        if (Julien->scopes_size < Julien_SCOPES_CAPACITY) {
            Julien->scopes[Julien->scopes_size].kind = kind;
            Julien->scopes[Julien->scopes_size].tail = 0;
            Julien->scopes[Julien->scopes_size].key = 0;
            Julien->scopes_size += 1;
        } else {
            Julien->error = Julien_SCOPES_OVERFLOW;
        }
    }
}

static void Julien_scope_pop(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        if (Julien->scopes_size > 0) {
            Julien->scopes_size--;
        } else {
            Julien->error = Julien_SCOPES_UNDERFLOW;
        }
    }
}

static Julien_Scope *Julien_current_scope(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        if (Julien->scopes_size > 0) {
            return &Julien->scopes[Julien->scopes_size - 1];
        }
    }

    return NULL;
}

static void Julien_write(Julien *Julien, const char *buffer, size_t size)
{
    if (Julien->error == Julien_OK) {
        if (Julien->write(buffer, 1, size, Julien->sink) < size) {
            Julien->error = 1;
        }
    }
}

static void Julien_write_cstr(Julien *Julien, const char *cstr)
{
    if (Julien->error == Julien_OK) {
        Julien_write(Julien, cstr, Julien_strlen(cstr));
    }
}

static int Julien_get_utf8_char_len(unsigned char ch)
{
    if ((ch & 0x80) == 0) return 1;
    switch (ch & 0xf0) {
    case 0xf0:
        return 4;
    case 0xe0:
        return 3;
    default:
        return 2;
    }
}

void Julien_element_begin(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_Scope *scope = Julien_current_scope(Julien);
        if (scope && scope->tail && !scope->key) {
            Julien_write_cstr(Julien, ",");
        }
    }
}

void Julien_element_end(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_Scope *scope = Julien_current_scope(Julien);
        if (scope) {
            scope->tail = 1;
            scope->key = 0;
        }
    }
}

const char *Julien_error_string(Julien_Error error)
{
    // TODO(#1): error strings are not particularly useful
    switch (error) {
    case Julien_OK:
        return "There is no error. The developer of this software just had a case of \"Task failed successfully\" https://i.imgur.com/Bdb3rkq.jpg - Please contact the developer and tell them that they are very lazy for not checking errors properly.";
    case Julien_WRITE_ERROR:
        return "Write error";
    case Julien_SCOPES_OVERFLOW:
        return "Stack of Scopes Overflow";
    case Julien_SCOPES_UNDERFLOW:
        return "Stack of Scopes Underflow";
    case Julien_OUT_OF_SCOPE_KEY:
        return "Out of Scope key";
    case Julien_DOUBLE_KEY:
        return "Tried to set the member key twice";
    default:
        return NULL;
    }
}

void Julien_null(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_write_cstr(Julien, "null");
        Julien_element_end(Julien);
    }
}

void Julien_bool(Julien *Julien, int boolean)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        if (boolean) {
            Julien_write_cstr(Julien, "true");
        } else {
            Julien_write_cstr(Julien, "false");
        }
        Julien_element_end(Julien);
    }
}

static void Julien_integer_no_element(Julien *Julien, long long int x)
{
    if (Julien->error == Julien_OK) {
        if (x < 0) {
            Julien_write_cstr(Julien, "-");
            x = -x;
        }

        if (x == 0) {
            Julien_write_cstr(Julien, "0");
        } else {
            char buffer[64];
            size_t count = 0;

            while (x > 0) {
                buffer[count++] = (x % 10) + '0';
                x /= 10;
            }

            for (size_t i = 0; i < count / 2; ++i) {
                char t = buffer[i];
                buffer[i] = buffer[count - i - 1];
                buffer[count - i - 1] = t;
            }

            Julien_write(Julien, buffer, count);
        }

    }
}

void Julien_integer(Julien *Julien, long long int x)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_integer_no_element(Julien, x);
        Julien_element_end(Julien);
    }
}

static int is_nan_or_inf(double x)
{
    unsigned long long int mask = (1ULL << 11ULL) - 1ULL;
    return (((*(unsigned long long int*) &x) >> 52ULL) & mask) == mask;
}

void Julien_float(Julien *Julien, double x, int precision)
{
    if (Julien->error == Julien_OK) {
        if (is_nan_or_inf(x)) {
            Julien_null(Julien);
        } else {
            Julien_element_begin(Julien);

            Julien_integer_no_element(Julien, (long long int) x);
            x -= (double) (long long int) x;
            while (precision-- > 0) {
                x *= 10.0;
            }
            Julien_write_cstr(Julien, ".");

            long long int y = (long long int) x;
            if (y < 0) {
                y = -y;
            }
            Julien_integer_no_element(Julien, y);

            Julien_element_end(Julien);
        }
    }
}

static void Julien_string_sized_no_element(Julien *Julien, const char *str, size_t size)
{
    if (Julien->error == Julien_OK) {
        const char *hex_digits = "0123456789abcdef";
        const char *specials = "btnvfr";
        const char *p = str;
        size_t len = size;

        Julien_write_cstr(Julien, "\"");
        size_t cl;
        for (size_t i = 0; i < len; i++) {
            unsigned char ch = ((unsigned char *) p)[i];
            if (ch == '"' || ch == '\\') {
                Julien_write(Julien, "\\", 1);
                Julien_write(Julien, p + i, 1);
            } else if (ch >= '\b' && ch <= '\r') {
                Julien_write(Julien, "\\", 1);
                Julien_write(Julien, &specials[ch - '\b'], 1);
            } else if (0x20 <= ch && ch <= 0x7F) { // is printable
                Julien_write(Julien, p + i, 1);
            } else if ((cl = Julien_get_utf8_char_len(ch)) == 1) {
                Julien_write(Julien, "\\u00", 4);
                Julien_write(Julien, &hex_digits[(ch >> 4) % 0xf], 1);
                Julien_write(Julien, &hex_digits[ch % 0xf], 1);
            } else {
                Julien_write(Julien, p + i, cl);
                i += cl - 1;
            }
        }

        Julien_write_cstr(Julien, "\"");
    }
}

void Julien_string_sized(Julien *Julien, const char *str, size_t size)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_string_sized_no_element(Julien, str, size);
        Julien_element_end(Julien);
    }
}

void Julien_string(Julien *Julien, const char *str)
{
    if (Julien->error == Julien_OK) {
        Julien_string_sized(Julien, str, Julien_strlen(str));
    }
}

void Julien_array_begin(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_write_cstr(Julien, "[");
        Julien_scope_push(Julien, Julien_ARRAY_SCOPE);
    }
}


void Julien_array_end(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_write_cstr(Julien, "]");
        Julien_scope_pop(Julien);
        Julien_element_end(Julien);
    }
}

void Julien_object_begin(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_write_cstr(Julien, "{");
        Julien_scope_push(Julien, Julien_OBJECT_SCOPE);
    }
}

void Julien_member_key(Julien *Julien, const char *str)
{
    if (Julien->error == Julien_OK) {
        Julien_member_key_sized(Julien, str, Julien_strlen(str));
    }
}

void Julien_member_key_sized(Julien *Julien, const char *str, size_t size)
{
    if (Julien->error == Julien_OK) {
        Julien_element_begin(Julien);
        Julien_Scope *scope = Julien_current_scope(Julien);
        if (scope && scope->kind == Julien_OBJECT_SCOPE) {
            if (!scope->key) {
                Julien_string_sized_no_element(Julien, str, size);
                Julien_write_cstr(Julien, ":");
                scope->key = 1;
            } else {
                Julien->error = Julien_DOUBLE_KEY;
            }
        } else {
            Julien->error = Julien_OUT_OF_SCOPE_KEY;
        }
    }
}

void Julien_object_end(Julien *Julien)
{
    if (Julien->error == Julien_OK) {
        Julien_write_cstr(Julien, "}");
        Julien_scope_pop(Julien);
        Julien_element_end(Julien);
    }
}

#endif // Julien_IMPLEMENTATION