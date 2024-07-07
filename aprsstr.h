#ifndef _aprsstr_h
#define _aprsstr_h

#define APRS_MAXLEN 255
void aprs_gencrctab(void);
int aprsstr_mon2raw(const char *mon, char raw[], int raw_len);

char *aprs_senddata();

#endif