/* gui_string_util.c - String utilities used by the GUI system.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/gui_string_util.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>

/* Create a new NUL-terminated string on the heap as a substring of BUF.
   The range of buf included is the half-open interval [START,END).
   The index START is inclusive, END is exclusive.  */
char *
grub_new_substring (const char *buf,
                    grub_size_t start, grub_size_t end)
{
  if (end < start)
    return 0;
  grub_size_t len = end - start;
  char *s = grub_malloc (len + 1);
  if (! s)
    return 0;
  grub_memcpy (s, buf + start, len);
  s[len] = '\0';
  return s;
}

/* Eliminate "." and ".." path elements from PATH.  A new heap-allocated
   string is returned.  */
static char *
canonicalize_path (const char *path)
{
  int i;
  const char *p;
  char *newpath = 0;

  /* Count the path components in path.  */
  int components = 1;
  for (p = path; *p; p++)
    if (*p == '/')
      components++;

  char **path_array = grub_malloc (components * sizeof (*path_array));
  if (! path_array)
    return 0;

  /* Initialize array elements to NULL pointers; in case once of the
     allocations fails, the cleanup code can just call grub_free() for all
     pointers in the array.  */
  for (i = 0; i < components; i++)
    path_array[i] = 0;

  /* Parse the path into path_array.  */
  p = path;
  for (i = 0; i < components && p; i++)
    {
      /* Find the end of the path element.  */
      const char *end = grub_strchr (p, '/');
      if (!end)
        end = p + grub_strlen (p);

      /* Copy the element.  */
      path_array[i] = grub_new_substring (p, 0, end - p);
      if (! path_array[i])
        goto cleanup;

      /* Advance p to point to the start of the next element, or NULL.  */
      if (*end)
        p = end + 1;
      else
        p = 0;
    }

  /* Eliminate '.' and '..' elements from the path array.  */
  int newpath_length = 0;
  for (i = components - 1; i >= 0; --i)
    {
      if (! grub_strcmp (path_array[i], "."))
        {
          grub_free (path_array[i]);
          path_array[i] = 0;
        }
      else if (! grub_strcmp (path_array[i], "..")
               && i > 0)
        {
          /* Delete the '..' and the prior path element.  */
          grub_free (path_array[i]);
          path_array[i] = 0;
          --i;
          grub_free (path_array[i]);
          path_array[i] = 0;
        }
      else
        {
          newpath_length += grub_strlen (path_array[i]) + 1;
        }
    }

  /* Construct a new path string.  */
  newpath = grub_malloc (newpath_length + 1);
  if (! newpath)
    goto cleanup;

  newpath[0] = '\0';
  char *newpath_end = newpath;
  int first = 1;
  for (i = 0; i < components; i++)
    {
      char *element = path_array[i];
      if (element)
        {
          /* For all components but the first, prefix with a slash.  */
          if (! first)
            newpath_end = grub_stpcpy (newpath_end, "/");
          newpath_end = grub_stpcpy (newpath_end, element);
          first = 0;
        }
    }

cleanup:
  for (i = 0; i < components; i++)
    grub_free (path_array[i]);
  grub_free (path_array);

  return newpath;
}

/* Return a new heap-allocated string representing to absolute path
   to the file referred to by PATH.  If PATH is an absolute path, then
   the returned path is a copy of PATH.  If PATH is a relative path, then
   BASE is with PATH used to construct the absolute path.  */
char *
grub_resolve_relative_path (const char *base, const char *path)
{
  char *abspath;
  char *canonpath;
  char *p;
  grub_size_t l;

  /* If PATH is an absolute path, then just use it as is.  */
  if (path[0] == '/' || path[0] == '(')
    return canonicalize_path (path);

  abspath = grub_malloc (grub_strlen (base) + grub_strlen (path) + 3);
  if (! abspath)
    return 0;

  /* Concatenate BASE and PATH.  */
  p = grub_stpcpy (abspath, base);
  l = grub_strlen (abspath);
  if (l == 0 || abspath[l-1] != '/')
    {
      *p = '/';
      p++;
      *p = 0;
    }
  grub_stpcpy (p, path);

  canonpath = canonicalize_path (abspath);
  if (! canonpath)
    return abspath;

  grub_free (abspath);
  return canonpath;
}

/* Get the path of the directory where the file at FILE_PATH is located.
   FILE_PATH should refer to a file, not a directory.  The returned path
   includes a trailing slash.
   This does not handle GRUB "(hd0,0)" paths properly yet since it only
   looks at slashes.  */
char *
grub_get_dirname (const char *file_path)
{
  int i;
  int last_slash;

  last_slash = -1;
  for (i = grub_strlen (file_path) - 1; i >= 0; --i)
    {
      if (file_path[i] == '/')
        {
          last_slash = i;
          break;
        }
    }
  if (last_slash == -1)
    return grub_strdup ("/");

  return grub_new_substring (file_path, 0, last_slash + 1);
}

static __inline int
my_isxdigit (char c)
{
  return ((c >= '0' && c <= '9')
          || (c >= 'a' && c <= 'f')
          || (c >= 'A' && c <= 'F'));
}

static int
parse_hex_color_component (const char *s, unsigned start, unsigned end)
{
  unsigned len;
  char buf[3];

  len = end - start;
  /* Check the limits so we don't overrun the buffer.  */
  if (len < 1 || len > 2)
    return 0;

  if (len == 1)
    {
      buf[0] = s[start];   /* Get the first and only hex digit.  */
      buf[1] = buf[0];     /* Duplicate the hex digit.  */
    }
  else if (len == 2)
    {
      buf[0] = s[start];
      buf[1] = s[start + 1];
    }

  buf[2] = '\0';

  return grub_strtoul (buf, 0, 16);
}

/* Parse a color string of the form "r, g, b", "#RGB", "#RGBA",
   "#RRGGBB", or "#RRGGBBAA".  */
grub_err_t
grub_gui_parse_color (const char *s, grub_gui_color_t *color)
{
  grub_gui_color_t c;

  /* Skip whitespace.  */
  while (*s && grub_isspace (*s))
    s++;

  if (*s == '#')
    {
      /* HTML-style.  Number if hex digits:
         [6] #RRGGBB     [3] #RGB
         [8] #RRGGBBAA   [4] #RGBA  */

      s++;  /* Skip the '#'.  */
      /* Count the hexits to determine the format.  */
      int hexits = 0;
      const char *end = s;
      while (my_isxdigit (*end))
        {
          end++;
          hexits++;
        }

      /* Parse the color components based on the format.  */
      if (hexits == 3 || hexits == 4)
        {
          c.red = parse_hex_color_component (s, 0, 1);
          c.green = parse_hex_color_component (s, 1, 2);
          c.blue = parse_hex_color_component (s, 2, 3);
          if (hexits == 4)
            c.alpha = parse_hex_color_component (s, 3, 4);
          else
            c.alpha = 255;
        }
      else if (hexits == 6 || hexits == 8)
        {
          c.red = parse_hex_color_component (s, 0, 2);
          c.green = parse_hex_color_component (s, 2, 4);
          c.blue = parse_hex_color_component (s, 4, 6);
          if (hexits == 8)
            c.alpha = parse_hex_color_component (s, 6, 8);
          else
            c.alpha = 255;
        }
      else
        return grub_error (GRUB_ERR_BAD_ARGUMENT,
                           "invalid HTML-type color string `%s'", s);
    }
  else if (grub_isdigit (*s))
    {
      /* Comma separated decimal values.  */
      c.red = grub_strtoul (s, 0, 0);
      if ((s = grub_strchr (s, ',')) == 0)
        return grub_error (GRUB_ERR_BAD_ARGUMENT,
                           "missing 1st comma separator in color `%s'", s);
      s++;
      c.green = grub_strtoul (s, 0, 0);
      if ((s = grub_strchr (s, ',')) == 0)
        return grub_error (GRUB_ERR_BAD_ARGUMENT,
                           "missing 2nd comma separator in color `%s'", s);
      s++;
      c.blue = grub_strtoul (s, 0, 0);
      if ((s = grub_strchr (s, ',')) == 0)
        c.alpha = 255;
      else
        {
          s++;
          c.alpha = grub_strtoul (s, 0, 0);
        }
    }
  else
    {
      if (! grub_gui_get_named_color (s, &c))
        return grub_error (GRUB_ERR_BAD_ARGUMENT,
                           "invalid named color `%s'", s);
    }

  if (grub_errno == GRUB_ERR_NONE)
    *color = c;
  return grub_errno;
}
