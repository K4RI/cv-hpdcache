/******************************************************************************
 *                           O P E N / C A E S A R
 *-----------------------------------------------------------------------------
 *   INRIA - Unite de Recherche Rhone-Alpes
 *   655, avenue de l'Europe
 *   38330 Montbonnot Saint Martin
 *   FRANCE
 *-----------------------------------------------------------------------------
 *   Module             :       testor_labels.h
 *   Authors            :       R. MATEESCU and W. SERWE
 *   Version            :       1.3
 *   Date               :       2022/05/12 23:42:32
 *****************************************************************************/

#ifndef __TESTOR_LABELS__
#define __TESTOR_LABELS__

/* les etiquettes suivent les conventions de TGV */

/* etiquettes cachee */

#define TESTOR_HIDDEN "i"

/* etiquettes speciales d'un objectif de test */

#define TESTOR_ACCEPT "ACCEPT"
#define TESTOR_REFUSE "REFUSE"
#define TESTOR_OTHERWISE "*"

/* etiquettes speciales d'un automate de suspension */

#define TESTOR_DELTA ":DELTA:"

/* etiquettes speciales d'un cas de test */

#define TESTOR_FAIL ":FAIL:"
#define TESTOR_INCONCLUSIVE ":INCONCLUSIVE:"
#define TESTOR_PASS ":PASS:"

/* etiquettes speciales pour les alarmes d'un cas de test */

#define TESTOR_START_AC "START TAC"
#define TESTOR_CANCEL_AC "CANCEL TAC"
#define TESTOR_TIMEOUT_AC "TIMEOUT TAC"

#define TESTOR_START_NOAC "START TNOAC"
#define TESTOR_CANCEL_NOAC "CANCEL TNOAC"
#define TESTOR_TIMEOUT_NOAC "TIMEOUT TNOAC"

#endif
