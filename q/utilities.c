
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

#define unless(C) if(!(C))

#define CREATE_SIZE 1

extern byte * create_s(void)
{   byte * p = HEAD + (byte *) malloc(HEAD + CREATE_SIZE + 1);
    CAPACITY(p) = CREATE_SIZE;
    SET_SIZE(p, CREATE_SIZE);
    return p;
}

extern void lose_s(byte * p) { free(p - HEAD); }

extern int in_grouping(struct SN_env * z, byte * s, int min, int max)
{   if (z->c >= z->l) return 0;
    {   int ch = z->p[z->c];
        if
        (ch > max || (ch -= min) < 0 ||
         (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) return 0;
    }
    z->c++; return 1;
}

extern int in_grouping_b(struct SN_env * z, char * s, int min, int max)
{   if (z->c <= z->lb) return 0;
    {   int ch = z->p[z->c - 1];
        if
        (ch > max || (ch -= min) < 0 ||
         (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) return 0;
    }
    z->c--; return 1;
}

extern int out_grouping(struct SN_env * z, byte * s, int min, int max)
{   if (z->c >= z->l) return 0;
    {   int ch = z->p[z->c];
        unless
        (ch > max || (ch -= min) < 0 ||
         (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) return 0;
    }
    z->c++; return 1;
}

extern int out_grouping_b(struct SN_env * z, char * s, int min, int max)
{   if (z->c <= z->lb) return 0;
    {   int ch = z->p[z->c - 1];
        unless
        (ch > max || (ch -= min) < 0 ||
         (s[ch >> 3] & (0X1 << (ch & 0X7))) == 0) return 0;
    }
    z->c--; return 1;
}


extern int in_range(struct SN_env * z, int min, int max)
{   if (z->c >= z->l) return 0;
    {   int ch = z->p[z->c];
        if
        (ch > max || ch < min) return 0;
    }
    z->c++; return 1;
}

extern int in_range_b(struct SN_env * z, int min, int max)
{   if (z->c <= z->lb) return 0;
    {   int ch = z->p[z->c - 1];
        if
        (ch > max || ch < min) return 0;
    }
    z->c--; return 1;
}

extern int out_range(struct SN_env * z, int min, int max)
{   if (z->c >= z->l) return 0;
    {   int ch = z->p[z->c];
        unless
        (ch > max || ch < min) return 0;
    }
    z->c++; return 1;
}

extern int out_range_b(struct SN_env * z, int min, int max)
{   if (z->c <= z->lb) return 0;
    {   int ch = z->p[z->c - 1];
        unless
        (ch > max || ch < min) return 0;
    }
    z->c--; return 1;
}

extern int eq_s(struct SN_env * z, int s_size, char * s)
{   if (z->l - z->c < s_size ||
        memcmp(z->p + z->c, s, s_size) != 0) return 0;
    z->c += s_size; return 1;
}

extern int eq_s_b(struct SN_env * z, int s_size, char * s)
{   if (z->c - z->lb < s_size ||
        memcmp(z->p + z->c - s_size, s, s_size) != 0) return 0;
    z->c -= s_size; return 1;
}

extern int eq_v(struct SN_env * z, byte * p)
{   return eq_s(z, SIZE(p), (char *)p);
}

extern int eq_v_b(struct SN_env * z, byte * p)
{   return eq_s_b(z, SIZE(p), (char *)p);
}

extern int find_among(struct SN_env * z, struct among * v, int v_size)
{
    int i = 0;
    int j = v_size;

    int c = z->c; int l = z->l;
    byte * q = z->p + c;

    struct among * w;

    int common_i = 0;
    int common_j = 0;

    int first_key_inspected = 0;

    while(1)
    {   int k = i + ((j - i) >> 1);
        int diff = 0;
        int common = common_i < common_j ? common_i : common_j; /* smaller */
        w = v + k;
        {   int i; for (i = common; i < w->s_size; i++)
            {   if (c + common == l) { diff = -1; break; }
                diff = q[common] - w->s[i];
                if (diff != 0) break;
                common++;
            }
        }
        if (diff < 0) { j = k; common_j = common; }
                 else { i = k; common_i = common; }
        if (j - i <= 1)
        {   if (i > 0) break; /* v->s has been inspected */
            if (j == i) break; /* only one item in v */

            /* - but now we need to go round once more to get
               v->s inspected. This looks messy, but is actually
               the optimal approach.  */

            if (first_key_inspected) break;
            first_key_inspected = 1;
        }
    }
    while(1)
    {   w = v + i;
        if (common_i >= w->s_size) { z->c += w->s_size; return w->result; }
        i = w->substring_i;
        if (i < 0) return 0;
    }
}

/* find_among_b is for backwards processing. Same comments apply */

extern int find_among_b(struct SN_env * z, struct among * v, int v_size)
{
    int i = 0;
    int j = v_size;

    int c = z->c; int lb = z->lb;
    byte * q = z->p + c - 1;

    struct among * w;

    int common_i = 0;
    int common_j = 0;

    int first_key_inspected = 0;

    while(1)
    {   int k = i + ((j - i) >> 1);
        int diff = 0;
        int common = common_i < common_j ? common_i : common_j;
        w = v + k;
        {   int i; for (i = w->s_size - 1 - common; i >= 0; i--)
            {   if (c - common == lb) { diff = -1; break; }
                diff = q[- common] - w->s[i];
                if (diff != 0) break;
                common++;
            }
        }
        if (diff < 0) { j = k; common_j = common; }
                 else { i = k; common_i = common; }
        if (j - i <= 1)
        {   if (i > 0) break;
            if (j == i) break;
            if (first_key_inspected) break;
            first_key_inspected = 1;
        }
    }
    while(1)
    {   w = v + i;
        if (common_i >= w->s_size) { z->c -= w->s_size; return w->result; }
        i = w->substring_i;
        if (i < 0) return 0;
    }
}


extern byte * increase_size(byte * p, int n)
{   int new_size = /**-CAPACITY(p) +-**/ n + 20;
    byte * q = HEAD + malloc(HEAD + new_size + 1);
    CAPACITY(q) = new_size;
    memmove(q, p, CAPACITY(p)); lose_s(p); return q;
}

/* to replace chars between c_bra and c_ket in z->p by the
   s_size chars at s
*/

extern int replace_s(struct SN_env * z, int c_bra, int c_ket, int s_size, byte * s)
{   int adjustment = s_size - (c_ket - c_bra);
    int len = SIZE(z->p);
    if (adjustment != 0)
    {   if (adjustment + len > CAPACITY(z->p)) z->p = increase_size(z->p, adjustment + len);
        memmove(z->p + c_ket + adjustment, z->p + c_ket, len - c_ket);
        SET_SIZE(z->p, adjustment + len);
        z->l += adjustment;
        if (z->c >= c_ket) z->c += adjustment; else
            if (z->c > c_bra) z->c = c_bra;
    }
    unless (s_size == 0) memmove(z->p + c_bra, s, s_size);
    return adjustment;
}

static void slice_check(struct SN_env * z)
{
    if (!(0 <= z->bra &&
          z->bra <= z->ket &&
          z->ket <= z->l &&
          z->l <= SIZE(z->p)))   /* this line could be removed */
    {
        fprintf(stderr, "faulty slice operation:\n");
        debug(z, -1, 0);
        exit(1);
    }
}

extern void slice_from_s(struct SN_env * z, int s_size, char * s)
{   slice_check(z);
    replace_s(z, z->bra, z->ket, s_size, (byte *) s);
}

extern void slice_from_v(struct SN_env * z, byte * p)
{   slice_from_s(z, SIZE(p), (char *)p);
}

extern void slice_del(struct SN_env * z)
{   slice_from_s(z, 0, 0);
}

extern void insert_s(struct SN_env * z, int bra, int ket, int s_size, char * s)
{   int adjustment = replace_s(z, bra, ket, s_size, (byte *) s);
    if (bra <= z->bra) z->bra += adjustment;
    if (bra <= z->ket) z->ket += adjustment;
}

extern void insert_v(struct SN_env * z, int bra, int ket, byte * p)
{   int adjustment = replace_s(z, bra, ket, SIZE(p), p);
    if (bra <= z->bra) z->bra += adjustment;
    if (bra <= z->ket) z->ket += adjustment;
}

extern byte * slice_to(struct SN_env * z, byte * p)
{   slice_check(z);
    {   int len = z->ket - z->bra;
        if (CAPACITY(p) < len) p = increase_size(p, len);
        memmove(p, z->p + z->bra, len);
        SET_SIZE(p, len);
    }
    return p;
}

extern byte * assign_to(struct SN_env * z, byte * p)
{   int len = z->l;
    if (CAPACITY(p) < len) p = increase_size(p, len);
    memmove(p, z->p, len);
    SET_SIZE(p, len);
    return p;
}

extern void debug(struct SN_env * z, int number, int line_count)
{   int i;
    int limit = SIZE(z->p);
    if (number >= 0) printf("%3d (line %4d): '", number, line_count);
    for (i = 0; i <= limit; i++)
    {   if (z->lb == i) printf("{");
        if (z->bra == i) printf("[");
        if (z->c == i) printf("|");
        if (z->ket == i) printf("]");
        if (z->l == i) printf("}");
        if (i < limit) printf("%c", z->p[i]);
    }
    printf("'\n");
}
