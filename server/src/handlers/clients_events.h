#ifndef _HANDLERS_CLIENTS_EVENTS_H
#define _HANDLERS_CLIENTS_EVENTS_H

#include <gtk/gtk.h>
#include <stdio.h>

#include "../gui/env.h"

gboolean
clients_events_trigger(GtkWidget *treeview, gpointer userdata);

#endif
