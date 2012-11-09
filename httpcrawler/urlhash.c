#include "urlhash.h"

unsigned int sax_hash(const char *key) {
    unsigned int h=0;

    while(*key) h^=(h<<5)+(h>>2)+(unsigned char)*key++;

    return h;
}
