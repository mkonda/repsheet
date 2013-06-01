#ifndef __MOD_SECURITY_H
#define __MOD_SECURITY_H

int matches(char *waf_events);
void **process_mod_security_headers(char *waf_events, char *events[]);

#endif
