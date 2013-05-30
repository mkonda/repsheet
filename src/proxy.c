#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *process_headers(char *headers) {
  if (headers == NULL) {
    return NULL;
  }
  
  int matches;
  int subvector[30];
  int error_offset;

  const char *error;
  const char *address;

  pcre *re_compiled;
  
  char *regex = "([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})";

  re_compiled = pcre_compile(regex, 0, &error, &error_offset, NULL);

  matches = pcre_exec(re_compiled, 0, headers, strlen(headers), 0, 0, subvector, 30);

  if(matches < 0) {
    return NULL;
  } else {
    pcre_get_substring(headers, subvector, matches, 0, &(address));
  }

  pcre_free(re_compiled);

  return address;
}
