#include <libircclient.h>

int irc_cmd_who(irc_session_t *session, const char *nick){
    if (!nick){
        // session->lasterror = LIBIRC_ERR_STATE;
        return 1;
    }

	return irc_send_raw(session, "WHO %s", nick);
}

