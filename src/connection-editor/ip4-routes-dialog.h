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
 * (C) Copyright 2008 Red Hat, Inc.
 */

#ifndef IP4_ROUTES_DIALOG_H
#define IP4_ROUTES_DIALOG_H

#include <glib.h>
#include <gtk/gtk.h>

#include "nm-setting-ip4-config.h"

GtkWidget *ip4_routes_dialog_new (NMSettingIP4Config *s_ip4, gboolean automatic);

void ip4_routes_dialog_update_setting (GtkWidget *dialog, NMSettingIP4Config *s_ip4);

#endif /* IP4_ROUTES_DIALOG_H */
