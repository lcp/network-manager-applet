/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager Connection editor -- Connection editor for NetworkManager
 *
 * Dan Williams <dcbw@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * (C) Copyright 2009 Red Hat, Inc.
 */

#include <string.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "ce-polkit-button.h"

G_DEFINE_TYPE (CEPolkitButton, ce_polkit_button, GTK_TYPE_BUTTON)

#define CE_POLKIT_BUTTON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), CE_TYPE_POLKIT_BUTTON, CEPolkitButtonPrivate))

typedef struct {
	gboolean disposed;

	char *label;
	char *tooltip;
	char *auth_label;
	char *auth_tooltip;
	gboolean master_sensitive;

	GtkWidget *stock;
	GtkWidget *auth;

	NMClient *client;
	NMClientPermission permission;
	/* authorized = TRUE if either explicitly authorized or if the action
	 * could be performed if the user successfully authenticated to gain the
	 * authorization.
	 */
	gboolean authorized;

	guint perm_id;
} CEPolkitButtonPrivate;

enum {
	ACTIONABLE,
	AUTHORIZED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
update_button (CEPolkitButton *self, gboolean actionable)
{
	CEPolkitButtonPrivate *priv = CE_POLKIT_BUTTON_GET_PRIVATE (self);

	gtk_widget_set_sensitive (GTK_WIDGET (self), actionable);

	if (priv->authorized) {
		gtk_button_set_label (GTK_BUTTON (self), priv->auth_label);
		gtk_widget_set_tooltip_text (GTK_WIDGET (self), priv->auth_tooltip);
		gtk_button_set_image (GTK_BUTTON (self), priv->auth);
	} else {
		gtk_button_set_label (GTK_BUTTON (self), priv->label);
		gtk_widget_set_tooltip_text (GTK_WIDGET (self), priv->tooltip);
		gtk_button_set_image (GTK_BUTTON (self), priv->stock);
	}
}

static void
update_and_emit (CEPolkitButton *self, gboolean old_actionable)
{
	gboolean new_actionable;

	new_actionable = ce_polkit_button_get_actionable (self);
	update_button (self, new_actionable);
	if (new_actionable != old_actionable)
		g_signal_emit (self, signals[ACTIONABLE], 0, new_actionable);
}

void
ce_polkit_button_set_master_sensitive (CEPolkitButton *self, gboolean sensitive)
{
	gboolean old_actionable;

	g_return_if_fail (self != NULL);
	g_return_if_fail (CE_IS_POLKIT_BUTTON (self));

	old_actionable = ce_polkit_button_get_actionable (self);
	CE_POLKIT_BUTTON_GET_PRIVATE (self)->master_sensitive = sensitive;
	update_and_emit (self, old_actionable);
}

gboolean
ce_polkit_button_get_actionable (CEPolkitButton *self)
{
	CEPolkitButtonPrivate *priv;

	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (CE_IS_POLKIT_BUTTON (self), FALSE);

	priv = CE_POLKIT_BUTTON_GET_PRIVATE (self);

	return priv->master_sensitive && priv->authorized;
}

gboolean
ce_polkit_button_get_authorized (CEPolkitButton *self)
{
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (CE_IS_POLKIT_BUTTON (self), FALSE);

	return CE_POLKIT_BUTTON_GET_PRIVATE (self)->authorized;
}

static void
permission_changed_cb (NMClient *client,
                       NMClientPermission permission,
                       NMClientPermissionResult result,
                       CEPolkitButton *self)
{
	CEPolkitButtonPrivate *priv = CE_POLKIT_BUTTON_GET_PRIVATE (self);
	gboolean old_actionable, old_authorized;

	old_actionable = ce_polkit_button_get_actionable (self);
	old_authorized = priv->authorized;

	priv->authorized = (result == NM_CLIENT_PERMISSION_RESULT_YES || result == NM_CLIENT_PERMISSION_RESULT_AUTH);
	update_and_emit (self, old_actionable);

	if (priv->authorized != old_authorized)
		g_signal_emit (self, signals[AUTHORIZED], 0, priv->authorized);
}

GtkWidget *
ce_polkit_button_new (const char *label,
                      const char *tooltip,
                      const char *auth_label,
                      const char *auth_tooltip,
                      const char *stock_icon,
                      NMClient *client,
                      NMClientPermission permission)
{
	GObject *object;
	CEPolkitButtonPrivate *priv;

	object = g_object_new (CE_TYPE_POLKIT_BUTTON, NULL);
	if (!object)
		return NULL;

	priv = CE_POLKIT_BUTTON_GET_PRIVATE (object);

	priv->label = g_strdup (label);
	priv->tooltip = g_strdup (tooltip);
	priv->auth_label = g_strdup (auth_label);
	priv->auth_tooltip = g_strdup (auth_tooltip);
	priv->permission = permission;

	priv->client = g_object_ref (client);
	priv->perm_id = g_signal_connect (client,
	                                  "permission-changed",
	                                  G_CALLBACK (permission_changed_cb),
	                                  object);

	priv->stock = gtk_image_new_from_stock (stock_icon, GTK_ICON_SIZE_BUTTON);
	g_object_ref_sink (priv->stock);
	priv->auth = gtk_image_new_from_stock (GTK_STOCK_DIALOG_AUTHENTICATION, GTK_ICON_SIZE_BUTTON);
	g_object_ref_sink (priv->auth);

	update_button (CE_POLKIT_BUTTON (object),
	               ce_polkit_button_get_actionable (CE_POLKIT_BUTTON (object)));

	permission_changed_cb (client,
	                       permission,
	                       nm_client_get_permission_result (client, permission),
	                       CE_POLKIT_BUTTON (object));

	return GTK_WIDGET (object);
}

static void
dispose (GObject *object)
{
	CEPolkitButtonPrivate *priv = CE_POLKIT_BUTTON_GET_PRIVATE (object);

	if (priv->disposed == FALSE) {
		priv->disposed = TRUE;

		if (priv->perm_id)
			g_signal_handler_disconnect (priv->client, priv->perm_id);

		g_object_unref (priv->client);
		g_object_unref (priv->auth);
		g_object_unref (priv->stock);
	}

	G_OBJECT_CLASS (ce_polkit_button_parent_class)->dispose (object);
}

static void
finalize (GObject *object)
{
	CEPolkitButtonPrivate *priv = CE_POLKIT_BUTTON_GET_PRIVATE (object);

	g_free (priv->label);
	g_free (priv->auth_label);
	g_free (priv->tooltip);
	g_free (priv->auth_tooltip);

	G_OBJECT_CLASS (ce_polkit_button_parent_class)->finalize (object);
}

static void
ce_polkit_button_init (CEPolkitButton *self)
{
}

static void
ce_polkit_button_class_init (CEPolkitButtonClass *pb_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (pb_class);

	g_type_class_add_private (object_class, sizeof (CEPolkitButtonPrivate));

	object_class->dispose = dispose;
	object_class->finalize = finalize;

	signals[ACTIONABLE] = g_signal_new ("actionable",
	                                    G_OBJECT_CLASS_TYPE (object_class),
	                                    G_SIGNAL_RUN_FIRST,
	                                    G_STRUCT_OFFSET (CEPolkitButtonClass, actionable),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__BOOLEAN,
	                                    G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	signals[AUTHORIZED] = g_signal_new ("authorized",
	                                    G_OBJECT_CLASS_TYPE (object_class),
	                                    G_SIGNAL_RUN_FIRST,
	                                    G_STRUCT_OFFSET (CEPolkitButtonClass, authorized),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__BOOLEAN,
	                                    G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

