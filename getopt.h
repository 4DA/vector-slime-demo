#include <string.h>
#include <stdio.h>

int   opterr = 1,    /* if error message should be printed */
   optind = 1,    /* index into parent argv vector */
   optopt,        /* character checked for validity */
   optreset;      /* reset getopt */
char  *optarg;    /* argument associated with option */

#define  BADCH (int)'?'
#define  BADARG   (int)':'
#define  EMSG  ""

/*
 * getopt --
 * Parse argc/argv argument vector.
 */
int
getopt(int argc, char * const argv[], const char *optstring)
{
   static char *place = EMSG;    /* option letter processing */
   char *oli;           /* option letter list index */

   if (optreset || *place == 0) {      /* update scanning pointer */
      optreset = 0;
      place = argv[optind];
      if (optind >= argc || *place++ != '-') {
         /* Argument is absent or is not an option */
         place = EMSG;
         return (-1);
      }
      optopt = *place++;
      if (optopt == '-' && *place == 0) {
         /* "--" => end of options */
         ++optind;
         place = EMSG;
         return (-1);
      }
      if (optopt == 0) {
         /* Solitary '-', treat as a '-' option
            if the program (eg su) is looking for it. */
         place = EMSG;
         if (strchr(optstring, '-') == NULL)
            return -1;
         optopt = '-';
      }
   } else
      optopt = *place++;

   /* See if option letter is one the caller wanted... */
   if (optopt == ':' || (oli = (char *)strchr(optstring, optopt)) == NULL) {
      if (*place == 0)
         ++optind;
      if (opterr && *optstring != ':')
         (void)fprintf(stderr,
                                      "unknown option -- %cn", optopt);
      return (BADCH);
   }

   /* Does this option need an argument? */
   if (oli[1] != ':') {
      /* don't need argument */
      optarg = NULL;
      if (*place == 0)
         ++optind;
   } else {
      /* Option-argument is either the rest of this argument or the
         entire next argument. */
      if (*place)
         optarg = place;
      else if (argc > ++optind)
         optarg = argv[optind];
      else {
         /* option-argument absent */
         place = EMSG;
         if (*optstring == ':')
            return (BADARG);
         if (opterr)
            (void)fprintf(stderr,
                                        "option requires an argument -- %cn",
                                        optopt);
         return (BADCH);
      }
      place = EMSG;
      ++optind;
   }
   return (optopt);        /* return option letter */
}