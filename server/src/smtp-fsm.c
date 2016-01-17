/*  
 *  EDIT THIS FILE WITH CAUTION  (smtp-fsm.c)
 *  
 *  It has been AutoGen-ed  Thursday January 14, 2016 at 11:24:54 AM MSK
 *  From the definitions    smtp.tpl
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  copyright (c) 2001-2007 by Bruce Korb - all rights reserved
 *
 *  AutoFSM is free software copyrighted by Bruce Korb.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name ``Bruce Korb'' nor the name of any other
 *     contributor may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *  
 *  AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM
#include "server_types.h"
#include "smtp-fsm.h"
#include "smtp_proto.h"

#include <ctype.h>
#include <pcre.h>
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */
/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

/*
 *  This is the prototype for the callback routines.  They are to
 *  return the next state.  Normally, that should be the value of
 *  the "maybe_next" argument.
 */
typedef te_smtp_state (smtp_callback_t)(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt );

static smtp_callback_t
    smtp_do_DATA_WAIT_quit,
    smtp_do_DATA_WAIT_rset,
    smtp_do_DATA_WAIT_vrfy,
    smtp_do_INIT_quit,
    smtp_do_INIT_rset,
    smtp_do_INIT_vrfy,
    smtp_do_PARSE_CMD_quit,
    smtp_do_PARSE_CMD_rset,
    smtp_do_PARSE_CMD_vrfy,
    smtp_do_PARSE_DATA_quit,
    smtp_do_PARSE_DATA_rset,
    smtp_do_PARSE_DATA_vrfy,
    smtp_do_PROCESS_QUIT_quit,
    smtp_do_PROCESS_QUIT_rset,
    smtp_do_PROCESS_QUIT_vrfy,
    smtp_do_PROCESS_RSET_quit,
    smtp_do_PROCESS_RSET_rset,
    smtp_do_PROCESS_RSET_vrfy,
    smtp_do_PROCESS_VERIFY_quit,
    smtp_do_PROCESS_VERIFY_rset,
    smtp_do_PROCESS_VERIFY_vrfy,
    smtp_do_RCPT_BEGIN_quit,
    smtp_do_RCPT_BEGIN_rset,
    smtp_do_RCPT_BEGIN_vrfy,
    smtp_do_RCPT_MIDDLE_quit,
    smtp_do_RCPT_MIDDLE_rset,
    smtp_do_RCPT_MIDDLE_vrfy,
    smtp_do_SEQ_ERR_quit,
    smtp_do_SEQ_ERR_rset,
    smtp_do_SEQ_ERR_vrfy,
    smtp_do_STORE_MAIL_quit,
    smtp_do_STORE_MAIL_rset,
    smtp_do_STORE_MAIL_vrfy,
    smtp_do_ST_ERR_quit,
    smtp_do_ST_ERR_rset,
    smtp_do_ST_ERR_vrfy,
    smtp_do_TRANS_BEGIN_quit,
    smtp_do_TRANS_BEGIN_rset,
    smtp_do_TRANS_BEGIN_vrfy,
    smtp_do_data_wait_data,
    smtp_do_data_wait_data_end,
    smtp_do_data_wait_data_rcv,
    smtp_do_data_wait_ehlo,
    smtp_do_data_wait_err,
    smtp_do_data_wait_helo,
    smtp_do_data_wait_mail,
    smtp_do_data_wait_rcpt,
    smtp_do_init_data,
    smtp_do_init_data_rcv,
    smtp_do_init_ehlo,
    smtp_do_init_helo,
    smtp_do_init_mail,
    smtp_do_init_rcpt,
    smtp_do_invalid,
    smtp_do_parse_cmd_data,
    smtp_do_parse_cmd_ehlo,
    smtp_do_parse_cmd_err,
    smtp_do_parse_cmd_helo,
    smtp_do_parse_cmd_mail,
    smtp_do_parse_cmd_rcpt,
    smtp_do_parse_data_data,
    smtp_do_parse_data_data_end,
    smtp_do_parse_data_data_rcv,
    smtp_do_parse_data_ehlo,
    smtp_do_parse_data_helo,
    smtp_do_parse_data_mail,
    smtp_do_parse_data_rcpt,
    smtp_do_process_quit_ok,
    //smtp_do_process_rset_ok,
    //smtp_do_process_verify_ok,
    smtp_do_rcpt_begin_data,
    smtp_do_rcpt_begin_data_rcv,
    smtp_do_rcpt_begin_ehlo,
    smtp_do_rcpt_begin_helo,
    smtp_do_rcpt_begin_mail,
    smtp_do_rcpt_begin_rcpt,
    smtp_do_rcpt_middle_data,
    smtp_do_rcpt_middle_data_rcv,
    smtp_do_rcpt_middle_ehlo,
    smtp_do_rcpt_middle_helo,
    smtp_do_rcpt_middle_mail,
    smtp_do_rcpt_middle_rcpt,
    //smtp_do_seq_err_ok,
    //smtp_do_st_err_ok,
    smtp_do_store_mail_err,
    smtp_do_store_mail_ok,
    smtp_do_trans_begin_data,
    smtp_do_trans_begin_data_rcv,
    smtp_do_trans_begin_ehlo,
    smtp_do_trans_begin_helo,
    smtp_do_trans_begin_mail,
    smtp_do_trans_begin_rcpt;

/*
 *  This declares all the state transition handling routines
 */
typedef struct transition t_smtp_transition;
struct transition {
    te_smtp_state      next_state;
    smtp_callback_t*   trans_proc;
};

/*
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
static const t_smtp_transition
smtp_trans_table[ SMTP_STATE_CT ][ SMTP_EVENT_CT ] = {

  /* STATE 0:  SMTP_ST_INIT */
  { { SMTP_ST_PARSE_CMD, smtp_do_init_helo },       /* EVT:  HELO */
    { SMTP_ST_PARSE_CMD, smtp_do_init_ehlo },       /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_INIT_vrfy },  /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_INIT_rset },    /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_INIT_quit },    /* EVT:  QUIT */
    { SMTP_ST_SEQ_ERR, smtp_do_init_mail },         /* EVT:  MAIL */
    { SMTP_ST_SEQ_ERR, smtp_do_init_rcpt },         /* EVT:  RCPT */
    { SMTP_ST_SEQ_ERR, smtp_do_init_data },         /* EVT:  DATA */
    { SMTP_ST_SEQ_ERR, smtp_do_init_data_rcv },     /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 1:  SMTP_ST_PARSE_CMD */
  { { SMTP_ST_TRANS_BEGIN, smtp_do_parse_cmd_helo }, /* EVT:  HELO */
    { SMTP_ST_TRANS_BEGIN, smtp_do_parse_cmd_ehlo }, /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_PARSE_CMD_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_PARSE_CMD_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_PARSE_CMD_quit }, /* EVT:  QUIT */
    { SMTP_ST_RCPT_BEGIN, smtp_do_parse_cmd_mail }, /* EVT:  MAIL */
    { SMTP_ST_RCPT_MIDDLE, smtp_do_parse_cmd_rcpt }, /* EVT:  RCPT */
    { SMTP_ST_DATA_WAIT, smtp_do_parse_cmd_data },  /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_ST_ERR, smtp_do_parse_cmd_err }       /* EVT:  ERR */
  },

  /* STATE 2:  SMTP_ST_PROCESS_QUIT */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_PROCESS_QUIT_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_PROCESS_QUIT_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_PROCESS_QUIT_quit }, /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_DONE, smtp_do_process_quit_ok },      /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 3:  SMTP_ST_PROCESS_VERIFY */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_PROCESS_VERIFY_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_PROCESS_VERIFY_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_PROCESS_VERIFY_quit }, /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
	// TODO: check this!
   // { SMTP_ST_*, smtp_do_process_verify_ok },       /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 4:  SMTP_ST_PROCESS_RSET */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_PROCESS_RSET_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_PROCESS_RSET_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_PROCESS_RSET_quit }, /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
   // { SMTP_ST_*, smtp_do_process_rset_ok },         /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 5:  SMTP_ST_SEQ_ERR */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_SEQ_ERR_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_SEQ_ERR_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_SEQ_ERR_quit }, /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
   // { SMTP_ST_*, smtp_do_seq_err_ok },              /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 6:  SMTP_ST_ST_ERR */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_ST_ERR_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_ST_ERR_rset },  /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_ST_ERR_quit },  /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
   // { SMTP_ST_*, smtp_do_st_err_ok },               /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 7:  SMTP_ST_TRANS_BEGIN */
  { { SMTP_ST_SEQ_ERR, smtp_do_trans_begin_helo },  /* EVT:  HELO */
    { SMTP_ST_SEQ_ERR, smtp_do_trans_begin_ehlo },  /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_TRANS_BEGIN_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_TRANS_BEGIN_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_TRANS_BEGIN_quit }, /* EVT:  QUIT */
    { SMTP_ST_PARSE_CMD, smtp_do_trans_begin_mail }, /* EVT:  MAIL */
    { SMTP_ST_SEQ_ERR, smtp_do_trans_begin_rcpt },  /* EVT:  RCPT */
    { SMTP_ST_SEQ_ERR, smtp_do_trans_begin_data },  /* EVT:  DATA */
    { SMTP_ST_SEQ_ERR, smtp_do_trans_begin_data_rcv }, /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 8:  SMTP_ST_RCPT_BEGIN */
  { { SMTP_ST_SEQ_ERR, smtp_do_rcpt_begin_helo },   /* EVT:  HELO */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_begin_ehlo },   /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_RCPT_BEGIN_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_RCPT_BEGIN_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_RCPT_BEGIN_quit }, /* EVT:  QUIT */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_begin_mail },   /* EVT:  MAIL */
    { SMTP_ST_PARSE_CMD, smtp_do_rcpt_begin_rcpt }, /* EVT:  RCPT */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_begin_data },   /* EVT:  DATA */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_begin_data_rcv }, /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 9:  SMTP_ST_RCPT_MIDDLE */
  { { SMTP_ST_SEQ_ERR, smtp_do_rcpt_middle_helo },  /* EVT:  HELO */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_middle_ehlo },  /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_RCPT_MIDDLE_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_RCPT_MIDDLE_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_RCPT_MIDDLE_quit }, /* EVT:  QUIT */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_middle_mail },  /* EVT:  MAIL */
    { SMTP_ST_PARSE_CMD, smtp_do_rcpt_middle_rcpt }, /* EVT:  RCPT */
    { SMTP_ST_PARSE_CMD, smtp_do_rcpt_middle_data }, /* EVT:  DATA */
    { SMTP_ST_SEQ_ERR, smtp_do_rcpt_middle_data_rcv }, /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 10:  SMTP_ST_DATA_WAIT */
  { { SMTP_ST_SEQ_ERR, smtp_do_data_wait_helo },    /* EVT:  HELO */
    { SMTP_ST_SEQ_ERR, smtp_do_data_wait_ehlo },    /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_DATA_WAIT_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_DATA_WAIT_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_DATA_WAIT_quit }, /* EVT:  QUIT */
    { SMTP_ST_SEQ_ERR, smtp_do_data_wait_mail },    /* EVT:  MAIL */
    { SMTP_ST_SEQ_ERR, smtp_do_data_wait_rcpt },    /* EVT:  RCPT */
    { SMTP_ST_SEQ_ERR, smtp_do_data_wait_data },    /* EVT:  DATA */
    { SMTP_ST_PARSE_DATA, smtp_do_data_wait_data_rcv }, /* EVT:  DATA_RCV */
    { SMTP_ST_SEQ_ERR, smtp_do_data_wait_data_end }, /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_ST_ERR, smtp_do_data_wait_err }       /* EVT:  ERR */
  },

  /* STATE 11:  SMTP_ST_PARSE_DATA */
  { { SMTP_ST_SEQ_ERR, smtp_do_parse_data_helo },   /* EVT:  HELO */
    { SMTP_ST_SEQ_ERR, smtp_do_parse_data_ehlo },   /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_PARSE_DATA_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_PARSE_DATA_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_PARSE_DATA_quit }, /* EVT:  QUIT */
    { SMTP_ST_SEQ_ERR, smtp_do_parse_data_mail },   /* EVT:  MAIL */
    { SMTP_ST_SEQ_ERR, smtp_do_parse_data_rcpt },   /* EVT:  RCPT */
    { SMTP_ST_SEQ_ERR, smtp_do_parse_data_data },   /* EVT:  DATA */
    { SMTP_ST_DATA_WAIT, smtp_do_parse_data_data_rcv }, /* EVT:  DATA_RCV */
    { SMTP_ST_STORE_MAIL, smtp_do_parse_data_data_end }, /* EVT:  DATA_END */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  OK */
    { SMTP_ST_INVALID, smtp_do_invalid }            /* EVT:  ERR */
  },

  /* STATE 12:  SMTP_ST_STORE_MAIL */
  { { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  HELO */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  EHLO */
    { SMTP_ST_PROCESS_VERIFY, smtp_do_STORE_MAIL_vrfy }, /* EVT:  VRFY */
    { SMTP_ST_PROCESS_RSET, smtp_do_STORE_MAIL_rset }, /* EVT:  RSET */
    { SMTP_ST_PROCESS_QUIT, smtp_do_STORE_MAIL_quit }, /* EVT:  QUIT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  MAIL */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  RCPT */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_RCV */
    { SMTP_ST_INVALID, smtp_do_invalid },           /* EVT:  DATA_END */
    { SMTP_ST_INIT, smtp_do_store_mail_ok },        /* EVT:  OK */
    { SMTP_ST_ST_ERR, smtp_do_store_mail_err }      /* EVT:  ERR */
  }
};


#define SmtpFsmErr_off     19
#define SmtpEvInvalid_off  75
#define SmtpStInit_off     83


static char const zSmtpStrings[286] =
    "** OUT-OF-RANGE **\0"
    "FSM Error:  in state %d (%s), event %d (%s) is invalid\n\0"
    "invalid\0"
    "init\0"
    "parse_cmd\0"
    "process_quit\0"
    "process_verify\0"
    "process_rset\0"
    "seq_err\0"
    "st_err\0"
    "trans_begin\0"
    "rcpt_begin\0"
    "rcpt_middle\0"
    "data_wait\0"
    "parse_data\0"
    "store_mail\0"
    "helo\0"
    "ehlo\0"
    "vrfy\0"
    "rset\0"
    "quit\0"
    "mail\0"
    "rcpt\0"
    "data\0"
    "data_rcv\0"
    "data_end\0"
    "ok\0"
    "err\0";

static const size_t aszSmtpStates[13] = {
    83,  88,  98,  111, 126, 139, 147, 154, 166, 177, 189, 199, 210 };

static const size_t aszSmtpEvents[13] = {
    221, 226, 231, 236, 241, 246, 251, 256, 261, 270, 279, 282, 75 };


#define SMTP_EVT_NAME(t)   ( (((unsigned)(t)) >= 13) \
    ? zSmtpStrings : zSmtpStrings + aszSmtpEvents[t])

#define SMTP_STATE_NAME(s) ( (((unsigned)(s)) >= 13) \
    ? zSmtpStrings : zSmtpStrings + aszSmtpStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

static int smtp_invalid_transition( te_smtp_state st, te_smtp_event evt );

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */

static const char *cmd_parse(pcre *re, const char *data, int data_len, int *len) {
	int ovec[24];
	int ovecsize = sizeof(ovec);

	int rc = pcre_exec(re, 0, data, (int)data_len, 0, 0, ovec, ovecsize);

	int off = ovec[0];
	*len = ovec[1] - ovec[0];
	if (rc < 0) {
		slog_e("Invalid command came, data == '%.*s'", (int)data_len, data);
		*len = -1;
		return NULL;
	}

	if (*len == 0) {
		slog_d("%s", "Empty addr found cmd");
		return "";
	}

	slog_d("smtp_data: cmd_parsed: '%.*s'", *len, data + off);
	return data + off;
}

static te_smtp_event parse_cmd_internal(struct smtp_data *s,
					const char **data, int *len)
{
	struct smtp_cmd_info *info = &smtp_cmd_arr[s->cur_cmd];
	slog_d("TEST: parse cmd : cmd %d", s->cur_cmd);

	if (info->re.re == NULL)
		return SMTP_EV_ERR;

	const char *args = cmd_parse(info->re.re, *data, *len, len);
	if (args == NULL) {
		*len = 0;
		*data = NULL;

		slog_d("incorret args for cmd %d", s->cur_cmd);
		return SMTP_EV_ERR;
	}

	slog_d("accepted cmd and args `%s'", args);
	*data = args;

	return SMTP_EV_OK;
}

static int
smtp_invalid_transition( te_smtp_state st, te_smtp_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char const * fmt = zSmtpStrings + SmtpFsmErr_off;
    fprintf( stderr, fmt, st, SMTP_STATE_NAME(st), evt, SMTP_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

static te_smtp_state
smtp_do_DATA_WAIT_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_DATA_WAIT_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_DATA_WAIT_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_INIT_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_INIT_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_INIT_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_CMD_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_CMD_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_CMD_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_DATA_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_DATA_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PARSE_DATA_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_QUIT_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS QUIT QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS QUIT QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_QUIT_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS QUIT RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS QUIT RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_QUIT_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS QUIT VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS QUIT VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_RSET_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS RSET QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS RSET QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_RSET_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS RSET RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS RSET RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_RSET_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS RSET VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS RSET VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_VERIFY_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS VERIFY QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS VERIFY QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_VERIFY_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS VERIFY RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS VERIFY RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_PROCESS_VERIFY_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS VERIFY VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS VERIFY VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_BEGIN_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_BEGIN_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_BEGIN_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_MIDDLE_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_MIDDLE_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_RCPT_MIDDLE_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_SEQ_ERR_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == SEQ ERR QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == SEQ ERR QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_SEQ_ERR_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == SEQ ERR RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == SEQ ERR RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_SEQ_ERR_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == SEQ ERR VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == SEQ ERR VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_STORE_MAIL_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == STORE MAIL QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == STORE MAIL QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_STORE_MAIL_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == STORE MAIL RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == STORE MAIL RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_STORE_MAIL_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == STORE MAIL VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == STORE MAIL VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_ST_ERR_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == ST ERR QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == ST ERR QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_ST_ERR_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == ST ERR RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == ST ERR RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_ST_ERR_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == ST ERR VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == ST ERR VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_TRANS_BEGIN_quit(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN QUIT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_TRANS_BEGIN_rset(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN RSET == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_TRANS_BEGIN_vrfy(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN VRFY == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN VRFY == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_data_end(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT DATA END == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT DATA END == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_err(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT ERR == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT ERR == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT HELO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_data_wait_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == DATA WAIT RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == DATA WAIT RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT HELO == DO NOT CHANGE THIS COMMENT  */
	slog_d("TEST: do init helo: initial %s trans %s next %s",
		SMTP_STATE_NAME(initial),
		SMTP_EVT_NAME(trans_evt),
		SMTP_STATE_NAME(maybe_next));

	struct smtp_data *s = (struct smtp_data *)data;
	s->answer.ret_msg_len = 0;

	te_smtp_state next = smtp_step(SMTP_ST_PARSE_CMD, trans_evt, data);

	if (s->answer.ret_msg_len == 0)
		SMTP_DATA_FORM_ANSWER(s, 250, s->name);

	slog_d("TEST: next state %s", SMTP_STATE_NAME(next));
    return next;
/*  END   == INIT HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_init_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INIT RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == INIT RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_invalid(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == INVALID == DO NOT CHANGE THIS COMMENT  */
    exit( smtp_invalid_transition( initial, trans_evt ));
/*  END   == INVALID == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_err(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD ERR == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD ERR == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD HELO == DO NOT CHANGE THIS COMMENT  */
	slog_d("%s", "TEST: parse cmd helo");

	struct smtp_data *s = (struct smtp_data*)data;
	const char *domain = s->client.data;
	int len = s->client.len;

	te_smtp_event evt = parse_cmd_internal(s, &domain, &len);

	if (evt == SMTP_EV_ERR) {
		char *err_msg = "incorrect domain argument";
		SMTP_DATA_FORM_ANSWER(s, 501, err_msg);

		return initial;
	}

	s->client.domain = strndup(domain, len);
	if (s->client.domain == NULL)
		abort();

	slog_d("TEST: parse cmd helo: next state %s", SMTP_STATE_NAME(maybe_next));
    return maybe_next;
/*  END   == PARSE CMD HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_cmd_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE CMD RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE CMD RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_data_end(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA DATA END == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA DATA END == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA HELO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_parse_data_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PARSE DATA RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PARSE DATA RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_process_quit_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == PROCESS QUIT OK == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == PROCESS QUIT OK == DO NOT CHANGE THIS COMMENT  */
}

/*
static te_smtp_state
smtp_do_process_rset_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{*/
/*  START == PROCESS RSET OK == DO NOT CHANGE THIS COMMENT  */
//    return maybe_next;
/*  END   == PROCESS RSET OK == DO NOT CHANGE THIS COMMENT  */
//}

/*
static te_smtp_state
smtp_do_process_verify_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{*/
/*  START == PROCESS VERIFY OK == DO NOT CHANGE THIS COMMENT  */
//    return maybe_next;
/*  END   == PROCESS VERIFY OK == DO NOT CHANGE THIS COMMENT  */
//}

static te_smtp_state
smtp_do_rcpt_begin_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_begin_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_begin_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_begin_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN HELO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_begin_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_begin_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT BEGIN RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT BEGIN RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE HELO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_rcpt_middle_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == RCPT MIDDLE RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == RCPT MIDDLE RCPT == DO NOT CHANGE THIS COMMENT  */
}

/*
static te_smtp_state
smtp_do_seq_err_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{*/
/*  START == SEQ ERR OK == DO NOT CHANGE THIS COMMENT  */
//    return maybe_next;
/*  END   == SEQ ERR OK == DO NOT CHANGE THIS COMMENT  */
//}

/*
static te_smtp_state
smtp_do_st_err_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{*/
/*  START == ST ERR OK == DO NOT CHANGE THIS COMMENT  */

//    return maybe_next;
/*  END   == ST ERR OK == DO NOT CHANGE THIS COMMENT  */
//}

static te_smtp_state
smtp_do_store_mail_err(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == STORE MAIL ERR == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == STORE MAIL ERR == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_store_mail_ok(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == STORE MAIL OK == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == STORE MAIL OK == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_data(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN DATA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_data_rcv(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN DATA RCV == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN DATA RCV == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_ehlo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN EHLO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_helo(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN HELO == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_mail(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN MAIL == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_state
smtp_do_trans_begin_rcpt(
    void *data,
    te_smtp_state initial,
    te_smtp_state maybe_next,
    te_smtp_event trans_evt )
{
/*  START == TRANS BEGIN RCPT == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == TRANS BEGIN RCPT == DO NOT CHANGE THIS COMMENT  */
}

/*
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  SMTP_ST_DONE or SMTP_ST_INVALID, it resets to
 *  SMTP_ST_INIT and returns SMTP_ST_INIT.
 */
te_smtp_state
smtp_step(
    te_smtp_state smtp_state,
    te_smtp_event trans_evt,
    void *data )
{
    te_smtp_state nxtSt;
    smtp_callback_t* pT;

    if ((unsigned)smtp_state >= SMTP_ST_INVALID) {
        return SMTP_ST_INIT;
    }

    if (trans_evt >= SMTP_EV_INVALID) {
        nxtSt = SMTP_ST_INVALID;
        pT    = smtp_do_invalid;
    } else {
        const t_smtp_transition* pTT =
            smtp_trans_table[ smtp_state ] + trans_evt;
        nxtSt = pTT->next_state;
        pT    = pTT->trans_proc;
    }

    if (pT != NULL)
        nxtSt = (*pT)( data, smtp_state, nxtSt, trans_evt );



    /* START == FINISH STEP == DO NOT CHANGE THIS COMMENT */
    /* END   == FINISH STEP == DO NOT CHANGE THIS COMMENT */

    return nxtSt;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of smtp-fsm.c */
