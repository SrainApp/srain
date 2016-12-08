#ifndef __FILTER_H
#define __FILTER_H

#define FILTER_MAX_LEN 50
#define FILTER_NAME_MAX_LEN 50

typedef struct {
    char name[FILTER_NAME_MAX_LEN];				//Filter's name
    char channel_name[32];		//Channel affected by this filter
    char regex[FILTER_MAX_LEN];			//Regex
} FilterItem;

int filter_ignore_list_add(const char *nick);
int filter_ignore_list_rm(const char *nick);
int filter_relaybot_list_add(const char *nick, const char *ldelim, const char *rdelim);
int filter_relaybot_list_rm(const char *nick);
int filter_is_ignore(const char *nick);
void filter_relaybot_trans(const char *orgin_nick, char *nick, char *msg);

int filter_filter_add_filter(const char *name, const char *regex, const char *channel_name);
int filter_filter_remove_filter(const char *filter_name);
int filter_filter_show();
int filter_filter_check_message(const char *channel_name, const char *nick_name, const char *message);


#endif /* __FILTER_H */
