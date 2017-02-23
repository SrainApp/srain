#ifndef __SRAIN_ENTRY_COMPLETION_H
#define __SRAIN_ENTRY_COMPLETION_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_ENTRY_COMPLETION (srain_entry_completion_get_type())
#define SRAIN_ENTRY_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_ENTRY_COMPLETION, SrainEntryCompletion))
#define SRAIN_IS_ENTRY_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_ENTRY_COMPLETION))

typedef enum {
    KEYWORD_NORMAL,
    KEYWORD_TMP,
} SECKeywordType;

typedef struct _SrainEntryCompletion SrainEntryCompletion;
typedef struct _SrainEntryCompletionClass SrainEntryCompletionClass;

GType srain_entry_completion_get_type(void);
SrainEntryCompletion *srain_entry_completion_new(GtkEntry *entry);
int srain_entry_completion_add_keyword(SrainEntryCompletion *comp, const char *keyword, SECKeywordType type);
int srain_entry_completion_rm_keyword(SrainEntryCompletion *comp, const char *keyword);
void srain_entry_completion_complete(SrainEntryCompletion *completion);

#endif /* __SRAIN_ENTRY_COMPLETION_H */
