
#ifndef __goog_marshal_MARSHAL_H__
#define __goog_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT,STRING (./googmarshal.list:1) */
extern void goog_marshal_VOID__INT_STRING (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

/* VOID:STRING (./googmarshal.list:2) */
#define goog_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

G_END_DECLS

#endif /* __goog_marshal_MARSHAL_H__ */

