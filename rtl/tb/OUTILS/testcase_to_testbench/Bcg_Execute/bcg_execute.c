/******************************************************************************
 *                                B C G
 *-----------------------------------------------------------------------------
 *   INRIA - Unite de Recherche Rhone-Alpes
 *   655, avenue de l'Europe
 *   38330 Montbonnot Saint Martin
 *   FRANCE
 *-----------------------------------------------------------------------------
 *   Module             :       bcg_execute.c
 *   Auteurs            :       Wendelin SERWE
 *   Version            :       1.46
 *   Date               :       2022/07/05 17:36:33
 *****************************************************************************/

char BCG_WHAT_BCG_EXECUTE[] = "@(#)BCG -- 1.46 -- 2022/07/05 17:36:33 -- bcg_execute.c";

#if defined (ARCHITECTURE_PC_WIN32) || defined (ARCHITECTURE_PC_WIN64)
#define _POSIX
#define __USE_MINGW_ALARM
#define _BSD_SOURCE
#endif

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "bcg_user.h"
#include "caesar_hide_1.h"

#include "testor_labels.h"

/* ========================================================================= */

#ifdef ARCHITECTURE_PC_WIN32

/* on remplace random() et srandom() par rand() et srand(), respectivement */

#define random rand
#define srandom srand

#endif

/* ========================================================================= */

#define VERSION "1.1"

#define streq(s1,s2) (strcmp ((s1), (s2)) == 0)

static BCG_TYPE_C_STRING tool;
static BCG_TYPE_C_STRING variant;
static BCG_TYPE_BCG_FILE diagnostic;

/*---------------------------------------------------------------------------*/

/* graphe BCG */
static BCG_TYPE_OBJECT_TRANSITION bcg_graph;

/* masque pour identifier les etiquettes correspondant a une entree */
static CAESAR_TYPE_HIDE_1 match_input = NULL;

#define IS_INPUT(L) !CAESAR_TEST_HIDE_1 (match_input, (L))

/*---------------------------------------------------------------------------*/

#define ABORT(...) \
	{ \
		fprintf (stderr, "%s%s: error: ", tool, variant); \
		fprintf (stderr, __VA_ARGS__); \
		fprintf (stderr, "\n"); \
		exit (1); \
	}


/*---------------------------------------------------------------------------*/

static void TIMEOUT (int signal)
{
     if (signal != SIGALRM)
	  ABORT ("unexpected signal");
     fprintf (stderr, "%s (unexpected quiescence)\n", TESTOR_FAIL);
     BCG_OT_READ_BCG_END (&bcg_graph);
     exit (0);
}

/*---------------------------------------------------------------------------*/

static void show_timeout (BCG_TYPE_GENUINE_UINT timeout)
{
     fprintf (stderr,
	      "using timeout of %u second%s\n",
	      timeout,
	      (timeout == 1) ? "" : "s");
}

/* ========================================================================= */

int  main (int argc, char **argv)
{
     BCG_TYPE_FILE_NAME diagnostic_filename;
     BCG_TYPE_GENUINE_UINT timeout;
     CAESAR_TYPE_STRING option;
     CAESAR_TYPE_NATURAL match_kind;
     BCG_TYPE_GENUINE_UINT seed;
#ifndef ARCHITECTURE_PC_WIN32
     struct sigaction act;
#endif
     BCG_TYPE_STATE_NUMBER current;
     BCG_TYPE_C_STRING input;
     size_t length;
     BCG_TYPE_EDGE_NUMBER nb_outputs;
     BCG_TYPE_LABEL_NUMBER label;
     BCG_TYPE_STATE_NUMBER next;
     BCG_TYPE_EDGE_NUMBER m;
     BCG_TYPE_EDGE_NUMBER n;
     BCG_TYPE_C_STRING s;
     ssize_t l;
     BCG_TYPE_BOOLEAN found_input;
     BCG_TYPE_C_STRING verdict;

     BCG_INIT ();

     tool = argv[0];
     variant = "";
     --argc;
     ++argv;

     diagnostic = NULL;
     diagnostic_filename = NULL;
     timeout = 0;

     while (argc > 0) {
	  if (streq (*argv, "-diag")) {
		--argc;
		++argv;
		if (argc == 1)
		ABORT ("missing file-name for option ``-diag''");
		diagnostic_filename = argv[0];
	  } else if (streq (*argv, "-io")) {
	       option = *argv;
	       --argc;
	       ++argv;
	       if (argc == 0)
		    ABORT ("missing file-name for option ``%s''", option);
	       match_kind = 2;	/* gate matching */
	       if (streq (*argv, "-total")) {
		    match_kind = 0;
		    --argc;
		    ++argv;
	       } else if (streq (*argv, "-partial")) {
		    match_kind = 1;
		    --argc;
		    ++argv;
	       } else if (streq (*argv, "-gate")) {
		    match_kind = 2;
		    --argc;
		    ++argv;
	       }
	       if (argc == 0)
		    ABORT ("missing file-name for option ``%s''", option);
	       CAESAR_CREATE_HIDE_1 (&match_input,
			   argv[0],
			   "input",
			   "output",
			   match_kind);
	       if (match_input == NULL)
		    ABORT ("wrong matching file ``%s'' for option ``%s''", argv[0], option);
	  } else if (streq (*argv, "-seed")) {
	       --argc;
	       ++argv;
	       if (argc == 0)
		    ABORT ("missing number for option ``-seed''");
	       errno = 0;
	       seed = (BCG_TYPE_GENUINE_UINT) atoi (argv[0]);
	       if (errno)
		    ABORT ("incorrect number for option ``-seed''");
#ifdef DEBUG
	       fprintf (stderr, "using seed %u\n", seed);
#endif
	       srandom (seed);
	  } else if (streq (*argv, "-timeout")) {
	       --argc;
	       ++argv;
	       if (argc == 0)
		    ABORT ("missing number for option ``-timeout''");
	       errno = 0;
	       timeout = (BCG_TYPE_GENUINE_UINT) atoi (argv[0]);
	       if (errno)
		    ABORT ("incorrect number for option ``-timeout''");
#ifdef DEBUG
	       show_timeout (timeout);
#endif
	  } else if (streq (*argv, "-version")) {
	       (void) fprintf (stdout, "%s\n", VERSION);
	       return (0);
	  } else if (*argv[0] == '-') {
	       fprintf (stderr, "unknown option ``%s''\n", *argv);
	       exit (1);
	  } else {
	       /* found a string corresponding to a file name */
	       break;
	  }
	  --argc;
	  ++argv;
     }
     if (argc < 1)
	  ABORT ("missing BCG filename");
     if (match_input == NULL)
	  ABORT ("missing option -io");

     variant = " (TC)";

     BCG_OT_READ_BCG_BEGIN (argv[0], &bcg_graph, 1);

     if (timeout) {
	  (void) signal (SIGALRM, TIMEOUT);
	  }
     if (diagnostic_filename != NULL) {
	  diagnostic = fopen (diagnostic_filename, "w");
	  if (diagnostic == NULL)
	       fprintf (stderr, "ignoring option -diag: %s\n", strerror (errno));
#ifdef DEBUG
     } else {
	  diagnostic = stderr;
#endif
     }
     current = BCG_OT_INITIAL_STATE (bcg_graph);
     input = NULL;
     length = 0;
     while ( /* CONSTCOND */ 1) {
#ifdef DEBUG
	  fprintf (diagnostic, "current == %lu\n", current);
#endif

	  /* comptage du nombre de sorties possibles */
	  nb_outputs = 0;
	  BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
	       /*
		     * recherche de verdicts ; dans un cas de test produit
		     * par TESTOR, un etat de verdict est marque par une
		     * transition self-loop
		     */
		    s = BCG_OT_LABEL_STRING (bcg_graph, label);
		    if (streq (s, TESTOR_INCONCLUSIVE) ||
			streq (s, TESTOR_PASS)) {
			 /* verdict atteint */
			 fprintf (stderr, "%s\n", s);
			 goto END;
		    }
	       if (!BCG_OT_LABEL_VISIBLE (bcg_graph, label))
		    ABORT ("unexpected internal transition");
	       if (!IS_INPUT (BCG_OT_LABEL_STRING (bcg_graph, label)))
		    nb_outputs++;
	  } BCG_OT_END_ITERATE;
#ifdef DEBUG
	  fprintf (diagnostic, "\t%lu input%s, %lu output%s\n", nb_inputs, (nb_inputs == 1) ? "" : "s", nb_outputs, (nb_outputs == 1) ? "" : "s");
#endif
	  /* priorite 1 : emission d'une sortie */
	  if (nb_outputs > 0) {
	       m = random () % nb_outputs;
	       n = 0;
	       BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
		    s = BCG_OT_LABEL_STRING (bcg_graph, label);
		    if (!IS_INPUT (s)) {
			 if (n == m) {
			      /*
			       * ecriture de la m-ieme sortie (sans espaces
			       * en debut de chaine et sans suffixe a la TGV
			       */
			      while (isspace ((int) (*s)))
				   s++;
			      if (diagnostic != NULL)
				   fprintf (diagnostic, "\"%s\"\n", s);
			      fprintf (stdout, "%s\n", s);
			      fflush (stdout);
			      current = next;
			      break;
			 }
			 n++;
		    }
	       } BCG_OT_END_ITERATE;
	       continue;
	  }
	  /* priorite 2 : attente d'une entree */
	  if (feof (stdin))
	       exit (0);
#ifdef DEBUG
	  fprintf (diagnostic, "no outputs: waiting for input\n");
#endif
	  if (timeout)
	       alarm (timeout);

	  l = getline (&input, &length, stdin);

	  if (feof (stdin))
	       exit (0);
	  if (l == (ssize_t) - 1)
	       ABORT ("input error");
	  if ((l > 0) && (input[l - 1] == '\n'))
	       input[l - 1] = '\0';
	  if (timeout)
	       alarm (0);

	  if (diagnostic != NULL)
	       fprintf (diagnostic, "\"%s\"\n", input);

	  found_input = BCG_FALSE;
	  verdict = NULL;	/* par precaution */
	  BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
	       s = BCG_OT_LABEL_STRING (bcg_graph, label);
	       if (IS_INPUT (s)) {
		    if (streq (s, input)) {
				current = next;
				found_input = BCG_TRUE;
				break;
		    }
	       }
	  } BCG_OT_END_ITERATE;
	  if (found_input)
	       continue;

	  fprintf (stderr, ":FAIL: (unexpected output \"%s\" in state #%lu)\n", input, current);
	  goto END;
     }

END:
     BCG_OT_READ_BCG_END (&bcg_graph);
     return (0);
}

/* ========================================================================= */
