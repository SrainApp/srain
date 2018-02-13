/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FILE_HELPER_H
#define __FILE_HELPER_H

char *get_theme_file(const char *fname);
char *get_pixmap_file(const char *fname);
char *get_plugin_file(const char *fname);
char *get_config_file(const char *fname);
char *get_system_config_file(const char *fname);
char *get_avatar_file(const char *fname);
char *create_log_file(const char *srv_name, const char *fname);
int create_user_file();

#endif /* __FILE_HELPER_H */
