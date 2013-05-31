#include <string.h>
#include <pcre.h>

int matches(char *waf_events)
{
  int match, error_offset;
  int offset = 0;
  int matches = 0;
  int ovector[100];

  const char *error;

  pcre *regex = pcre_compile("\\d{6}", PCRE_MULTILINE, &error, &error_offset, 0);

  while (offset < strlen(waf_events) && (match = pcre_exec(regex, 0, waf_events, strlen(waf_events), offset, 0, ovector, sizeof(ovector))) >= 0) {
    matches++;
    offset = ovector[1];
  }

  return matches;
}

void process_mod_security_headers(char *waf_events, char *events[])
{
  if (waf_events == NULL) {
    return NULL;
  }

  int i = 0;
  int matches = 0;
  int offset = 0;
  int count = 0;
  int match, error_offset;
  int ovector[100];

  char *prev_event;

  const char *event;
  const char *error;

  pcre *regex;

  regex = pcre_compile("\\d{6}", PCRE_MULTILINE, &error, &error_offset, 0);

  while (offset < strlen(waf_events) && (match = pcre_exec(regex, 0, waf_events, strlen(waf_events), offset, 0, ovector, sizeof(ovector))) >= 0) {
    for(i = 0; i < match; i++) {
      pcre_get_substring(waf_events, ovector, match, i, &(event));
      if (event != prev_event) {
        strcpy(events[count], event);
        prev_event = event;
      }
    }
    count++;
    offset = ovector[1];
  }
}
