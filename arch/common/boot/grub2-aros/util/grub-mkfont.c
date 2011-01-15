/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010 Free Software Foundation, Inc.
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

#include <config.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/emu/misc.h>
#include <grub/util/misc.h>
#include <grub/misc.h>
#include <grub/i18n.h>
#include <grub/fontformat.h>
#include <grub/font.h>
#include <grub/unicode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_TABLES_H
#include <freetype/ftsynth.h>

#undef __FTERRORS_H__
#define FT_ERROR_START_LIST   const char *ft_errmsgs[] = { 
#define FT_ERRORDEF(e, v, s)  [e] = s,
#define FT_ERROR_END_LIST     };
#include FT_ERRORS_H   

#include "progname.h"

#define GRUB_FONT_DEFAULT_SIZE		16

#define GRUB_FONT_RANGE_BLOCK		1024

struct grub_glyph_info
{
  struct grub_glyph_info *next;
  grub_uint32_t char_code;
  int width;
  int height;
  int x_ofs;
  int y_ofs;
  int device_width;
  int bitmap_size;
  grub_uint8_t *bitmap;
};

enum file_formats
{
  PF2,
  ASCII_BITMAPS,
  WIDTH_SPEC
};

#define GRUB_FONT_FLAG_BOLD		1
#define GRUB_FONT_FLAG_NOBITMAP		2
#define GRUB_FONT_FLAG_NOHINTING	4
#define GRUB_FONT_FLAG_FORCEHINT	8

struct grub_font_info
{
  char* name;
  int style;
  int desc;
  int asce;
  int size;
  int max_width;
  int max_height;
  int min_y;
  int max_y;
  int flags;
  int num_range;
  grub_uint32_t *ranges;
  struct grub_glyph_info *glyphs_unsorted;
  struct grub_glyph_info *glyphs_sorted;
  int num_glyphs;
};

static struct option options[] =
{
  {"output", required_argument, 0, 'o'},
  {"name", required_argument, 0, 'n'},
  {"index", required_argument, 0, 'i'},
  {"range", required_argument, 0, 'r'},
  {"size", required_argument, 0, 's'},
  {"desc", required_argument, 0, 'd'},
  {"asce", required_argument, 0, 'c'},
  {"bold", no_argument, 0, 'b'},
  {"no-bitmap", no_argument, 0, 0x100},
  {"no-hinting", no_argument, 0, 0x101},
  {"force-autohint", no_argument, 0, 'a'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {"verbose", no_argument, 0, 'v'},
  {"ascii-bitmaps", no_argument, 0, 0x102},
  {"width-spec", no_argument, 0, 0x103},
  {0, 0, 0, 0}
};

int font_verbosity;

static void
usage (int status)
{
  if (status)
    fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
  else
    printf ("\
Usage: %s [OPTIONS] FONT_FILES\n\
\nOptions:\n\
  -o, --output=FILE_NAME    set output file name\n\
  --ascii-bitmaps           save only the ASCII bitmaps\n\
  --width-spec              create width summary file\n\
  -i, --index=N             set face index\n\
  -r, --range=A-B[,C-D]     set font range\n\
  -n, --name=S              set font family name\n\
  -s, --size=N              set font size\n\
  -d, --desc=N              set font descent\n\
  -c, --asce=N              set font ascent\n\
  -b, --bold                convert to bold font\n\
  -a, --force-autohint      force autohint\n\
  --no-hinting              disable hinting\n\
  --no-bitmap               ignore bitmap strikes when loading\n\
  -h, --help                display this message and exit\n\
  -V, --version             print version information and exit\n\
  -v, --verbose             print verbose messages\n\
\n\
Report bugs to <%s>.\n", program_name, PACKAGE_BUGREPORT);

  exit (status);
}

void
add_pixel (grub_uint8_t **data, int *mask, int not_blank)
{
  if (*mask == 0)
    {
      (*data)++;
      **data = 0;
      *mask = 128;
    }

  if (not_blank)
    **data |= *mask;

  *mask >>= 1;
}

static void
add_glyph (struct grub_font_info *font_info, FT_UInt glyph_idx, FT_Face face,
	   grub_uint32_t char_code, int nocut)
{
  struct grub_glyph_info *glyph_info;
  int width, height;
  int cuttop, cutbottom, cutleft, cutright;
  grub_uint8_t *data;
  int mask, i, j, bitmap_size;
  FT_GlyphSlot glyph;
  int flag = FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
  FT_Error err;

  if (font_info->flags & GRUB_FONT_FLAG_NOBITMAP)
    flag |= FT_LOAD_NO_BITMAP;

  if (font_info->flags & GRUB_FONT_FLAG_NOHINTING)
    flag |= FT_LOAD_NO_HINTING;
  else if (font_info->flags & GRUB_FONT_FLAG_FORCEHINT)
    flag |= FT_LOAD_FORCE_AUTOHINT;

  err = FT_Load_Glyph (face, glyph_idx, flag);
  if (err)
    {
      printf ("Freetype Error %d loading glyph 0x%x for U+0x%x%s",
	      err, glyph_idx, char_code & GRUB_FONT_CODE_CHAR_MASK,
	      char_code & GRUB_FONT_CODE_RIGHT_JOINED
	      ? ((char_code & GRUB_FONT_CODE_LEFT_JOINED) ? " (medial)":
		 " (leftmost)")
	      : ((char_code & GRUB_FONT_CODE_LEFT_JOINED) ? " (rightmost)":
		 ""));

      if (err > 0 && err < (signed) ARRAY_SIZE (ft_errmsgs))
	printf (": %s\n", ft_errmsgs[err]);
      else
	printf ("\n");
      return;
    }

  glyph = face->glyph;

  if (font_info->flags & GRUB_FONT_FLAG_BOLD)
    FT_GlyphSlot_Embolden (glyph);

  if (glyph->next)
    printf ("%x\n", char_code);

  if (nocut)
    cuttop = cutbottom = cutleft = cutright = 0;
  else
    {
      for (cuttop = 0; cuttop < glyph->bitmap.rows; cuttop++)
	{
	  for (j = 0; j < glyph->bitmap.width; j++)
	    if (glyph->bitmap.buffer[j / 8 + cuttop * glyph->bitmap.pitch]
		& (1 << (7 - (j & 7))))
	      break;
	  if (j != glyph->bitmap.width)
	    break;
	}

      for (cutbottom = glyph->bitmap.rows - 1; cutbottom >= 0; cutbottom--)
	{
	  for (j = 0; j < glyph->bitmap.width; j++)
	    if (glyph->bitmap.buffer[j / 8 + cutbottom * glyph->bitmap.pitch]
		& (1 << (7 - (j & 7))))
	      break;
	  if (j != glyph->bitmap.width)
	    break;
	}
      cutbottom = glyph->bitmap.rows - 1 - cutbottom;
      if (cutbottom + cuttop >= glyph->bitmap.rows)
	cutbottom = 0;

      for (cutleft = 0; cutleft < glyph->bitmap.width; cutleft++)
	{
	  for (j = 0; j < glyph->bitmap.rows; j++)
	    if (glyph->bitmap.buffer[cutleft / 8 + j * glyph->bitmap.pitch]
		& (1 << (7 - (cutleft & 7))))
	      break;
	  if (j != glyph->bitmap.rows)
	    break;
	}
      for (cutright = glyph->bitmap.width - 1; cutright >= 0; cutright--)
	{
	  for (j = 0; j < glyph->bitmap.rows; j++)
	    if (glyph->bitmap.buffer[cutright / 8 + j * glyph->bitmap.pitch]
		& (1 << (7 - (cutright & 7))))
	      break;
	  if (j != glyph->bitmap.rows)
	    break;
	}
      cutright = glyph->bitmap.width - 1 - cutright;
      if (cutright + cutleft >= glyph->bitmap.width)
	cutright = 0;
    }

  width = glyph->bitmap.width - cutleft - cutright;
  height = glyph->bitmap.rows - cutbottom - cuttop;

  bitmap_size = ((width * height + 7) / 8);
  glyph_info = xmalloc (sizeof (struct grub_glyph_info));
  glyph_info->bitmap = xmalloc (bitmap_size);
  glyph_info->bitmap_size = bitmap_size;

  glyph_info->next = font_info->glyphs_unsorted;
  font_info->glyphs_unsorted = glyph_info;
  font_info->num_glyphs++;

  glyph_info->char_code = char_code;
  glyph_info->width = width;
  glyph_info->height = height;
  glyph_info->x_ofs = glyph->bitmap_left + cutleft;
  glyph_info->y_ofs = glyph->bitmap_top - height - cuttop;
  glyph_info->device_width = glyph->metrics.horiAdvance / 64;

  if (width > font_info->max_width)
    font_info->max_width = width;

  if (height > font_info->max_height)
    font_info->max_height = height;

  if (glyph_info->y_ofs < font_info->min_y && glyph_info->y_ofs > -font_info->size)
    font_info->min_y = glyph_info->y_ofs;

  if (glyph_info->y_ofs + height > font_info->max_y)
    font_info->max_y = glyph_info->y_ofs + height;

  mask = 0;
  data = &glyph_info->bitmap[0] - 1;
  for (j = cuttop; j < height + cuttop; j++)
    for (i = cutleft; i < width + cutleft; i++)
      add_pixel (&data, &mask,
		 glyph->bitmap.buffer[i / 8 + j * glyph->bitmap.pitch] &
		 (1 << (7 - (i & 7))));
}

struct glyph_replace *subst_rightjoin, *subst_leftjoin, *subst_medijoin;

struct glyph_replace
{
  struct glyph_replace *next;
  grub_uint32_t from, to;
};

/* TODO: sort glyph_replace and use binary search if necessary.  */
static void
add_char (struct grub_font_info *font_info, FT_Face face,
	  grub_uint32_t char_code, int nocut)
{
  FT_UInt glyph_idx;
  struct glyph_replace *cur;

  glyph_idx = FT_Get_Char_Index (face, char_code);
  if (!glyph_idx)
    return;
  add_glyph (font_info, glyph_idx, face, char_code, nocut);
  for (cur = subst_rightjoin; cur; cur = cur->next)
    if (cur->from == glyph_idx)
      {
	add_glyph (font_info, cur->to, face,
		   char_code | GRUB_FONT_CODE_RIGHT_JOINED, nocut);
	break;
      }
  if (!cur && char_code >= GRUB_UNICODE_ARABIC_START
      && char_code < GRUB_UNICODE_ARABIC_END)
    {
      int i;
      for (i = 0; grub_unicode_arabic_shapes[i].code; i++)
	if (grub_unicode_arabic_shapes[i].code == char_code
	    && grub_unicode_arabic_shapes[i].right_linked)
	  {
	    FT_UInt idx2;
	    idx2 = FT_Get_Char_Index (face, grub_unicode_arabic_shapes[i]
				      .right_linked);
	    if (idx2)
	      add_glyph (font_info, idx2, face,
			 char_code | GRUB_FONT_CODE_RIGHT_JOINED, nocut);
	    break;
	  }
	      
    }

  for (cur = subst_leftjoin; cur; cur = cur->next)
    if (cur->from == glyph_idx)
      {
	add_glyph (font_info, cur->to, face,
		   char_code | GRUB_FONT_CODE_LEFT_JOINED, nocut);
	break;
      }
  if (!cur && char_code >= GRUB_UNICODE_ARABIC_START
      && char_code < GRUB_UNICODE_ARABIC_END)
    {
      int i;
      for (i = 0; grub_unicode_arabic_shapes[i].code; i++)
	if (grub_unicode_arabic_shapes[i].code == char_code
	    && grub_unicode_arabic_shapes[i].left_linked)
	  {
	    FT_UInt idx2;
	    idx2 = FT_Get_Char_Index (face, grub_unicode_arabic_shapes[i]
				      .left_linked);
	    if (idx2)
	      add_glyph (font_info, idx2, face,
			 char_code | GRUB_FONT_CODE_LEFT_JOINED, nocut);
	    break;
	  }
	      
    }
  for (cur = subst_medijoin; cur; cur = cur->next)
    if (cur->from == glyph_idx)
      {
	add_glyph (font_info, cur->to, face,
		   char_code | GRUB_FONT_CODE_LEFT_JOINED
		   | GRUB_FONT_CODE_RIGHT_JOINED, nocut);
	break;
      }
  if (!cur && char_code >= GRUB_UNICODE_ARABIC_START
      && char_code < GRUB_UNICODE_ARABIC_END)
    {
      int i;
      for (i = 0; grub_unicode_arabic_shapes[i].code; i++)
	if (grub_unicode_arabic_shapes[i].code == char_code
	    && grub_unicode_arabic_shapes[i].both_linked)
	  {
	    FT_UInt idx2;
	    idx2 = FT_Get_Char_Index (face, grub_unicode_arabic_shapes[i]
				      .both_linked);
	    if (idx2)
	      add_glyph (font_info, idx2, face,
			 char_code | GRUB_FONT_CODE_LEFT_JOINED
			 | GRUB_FONT_CODE_RIGHT_JOINED, nocut);
	    break;
	  }
	      
    }
}

struct gsub_header
{
  grub_uint32_t version;
  grub_uint16_t scripts_off;
  grub_uint16_t features_off;
  grub_uint16_t lookups_off;
} __attribute__ ((packed));

struct gsub_features
{
  grub_uint16_t count;
  struct
  {
#define FEATURE_FINA 0x66696e61
#define FEATURE_INIT 0x696e6974
#define FEATURE_MEDI 0x6d656469
#define FEATURE_AALT 0x61616c74
#define FEATURE_LIGA 0x6c696761
#define FEATURE_RLIG 0x726c6967
    grub_uint32_t feature_tag;
    grub_uint16_t offset;
  } __attribute__ ((packed)) features[0];
} __attribute__ ((packed));

struct gsub_feature
{
  grub_uint16_t params;
  grub_uint16_t lookupcount;
  grub_uint16_t lookupindices[0];
} __attribute__ ((packed));

struct gsub_lookup_list
{
  grub_uint16_t count;
  grub_uint16_t offsets[0];
} __attribute__ ((packed));

struct gsub_lookup
{
  grub_uint16_t type;
  grub_uint16_t flag;
  grub_uint16_t subtablecount;
  grub_uint16_t subtables[0];
} __attribute__ ((packed));

struct gsub_substitution
{
  grub_uint16_t type;
  grub_uint16_t coverage_off;
  union
  {
    grub_int16_t delta;
    struct
    {
      grub_int16_t count;
      grub_uint16_t repl[0];
    };
  };
} __attribute__ ((packed));

struct gsub_coverage_list
{
  grub_uint16_t type;
  grub_uint16_t count;
  grub_uint16_t glyphs[0];
} __attribute__ ((packed));

struct gsub_coverage_ranges
{
  grub_uint16_t type;
  grub_uint16_t count;
  struct 
  {
    grub_uint16_t start;
    grub_uint16_t end;
    grub_uint16_t start_index;
  } __attribute__ ((packed)) ranges[0];
} __attribute__ ((packed));

#define GSUB_SINGLE_SUBSTITUTION 1

#define GSUB_SUBSTITUTION_DELTA 1
#define GSUB_SUBSTITUTION_MAP 2

#define GSUB_COVERAGE_LIST 1
#define GSUB_COVERAGE_RANGE 2

#define GSUB_RTL_CHAR 1

static void
add_subst (grub_uint32_t from, grub_uint32_t to, struct glyph_replace **target)
{
  struct glyph_replace *new = xmalloc (sizeof (*new));
  new->next = *target;
  new->from = from;
  new->to = to;
  *target = new;
}

static void
process_cursive (struct gsub_feature *feature,
		 struct gsub_lookup_list *lookups,
		 grub_uint32_t feattag)
{
  int j, k;
  int i;
  struct glyph_replace **target;
  struct gsub_substitution *sub;

  auto inline void subst (grub_uint32_t glyph);
  void subst (grub_uint32_t glyph)
  {
    grub_uint16_t substtype;
    substtype = grub_be_to_cpu16 (sub->type);

    if (substtype == GSUB_SUBSTITUTION_DELTA)
      add_subst (glyph, glyph + grub_be_to_cpu16 (sub->delta), target);
    else if (i >= grub_be_to_cpu16 (sub->count))
      printf ("Out of range substitution (%d, %d)\n", i,
	      grub_be_to_cpu16 (sub->count));
    else
      add_subst (glyph, grub_be_to_cpu16 (sub->repl[i++]), target);
  }

  for (j = 0; j < grub_be_to_cpu16 (feature->lookupcount); j++)
    {
      int lookup_index = grub_be_to_cpu16 (feature->lookupindices[j]);
      struct gsub_lookup *lookup;
      if (lookup_index >= grub_be_to_cpu16 (lookups->count))
	{
	  printf ("Out of range lookup: %d\n", lookup_index);
	  continue;
	}
      lookup = (struct gsub_lookup *)
	((grub_uint8_t *) lookups 
	 + grub_be_to_cpu16 (lookups->offsets[lookup_index]));
      if (grub_be_to_cpu16 (lookup->type) != GSUB_SINGLE_SUBSTITUTION)
	{
	  printf ("Unsupported substitution type: %d\n",
		  grub_be_to_cpu16 (lookup->type));
	  continue;
	}		      
      if (grub_be_to_cpu16 (lookup->flag) & ~GSUB_RTL_CHAR)
	{
	  printf ("Unsupported substitution flag: 0x%x\n",
		  grub_be_to_cpu16 (lookup->flag));
	}
      switch (feattag)
	{
	case FEATURE_INIT:
	  if (grub_be_to_cpu16 (lookup->flag) & GSUB_RTL_CHAR)
	    target = &subst_leftjoin;
	  else
	    target = &subst_rightjoin;
	  break;
	case FEATURE_FINA:
	  if (grub_be_to_cpu16 (lookup->flag) & GSUB_RTL_CHAR)
	    target = &subst_rightjoin;
	  else
	    target = &subst_leftjoin;
	  break;
	case FEATURE_MEDI:
	  target = &subst_medijoin;
	  break;	  
	}
      for (k = 0; k < grub_be_to_cpu16 (lookup->subtablecount); k++)
	{
	  sub = (struct gsub_substitution *)
	    ((grub_uint8_t *) lookup + grub_be_to_cpu16 (lookup->subtables[k]));
	  grub_uint16_t substtype;
	  substtype = grub_be_to_cpu16 (sub->type);
	  if (substtype != GSUB_SUBSTITUTION_MAP
	      && substtype != GSUB_SUBSTITUTION_DELTA)
	    {
	      printf ("Unsupported substitution specification: %d\n",
		      substtype);
	      continue;
	    }
	  void *coverage = (grub_uint8_t *) sub
	    + grub_be_to_cpu16 (sub->coverage_off);
	  grub_uint32_t covertype;
	  covertype = grub_be_to_cpu16 (*(grub_uint16_t * __attribute__ ((packed))) coverage);
	  i = 0;
	  if (covertype == GSUB_COVERAGE_LIST)
	    {
	      struct gsub_coverage_list *cover = coverage;
	      int l;
	      for (l = 0; l < grub_be_to_cpu16 (cover->count); l++)
		subst (grub_be_to_cpu16 (cover->glyphs[l]));
	    }
	  else if (covertype == GSUB_COVERAGE_RANGE)
	    {
	      struct gsub_coverage_ranges *cover = coverage;
	      int l, m;
	      for (l = 0; l < grub_be_to_cpu16 (cover->count); l++)
		for (m = grub_be_to_cpu16 (cover->ranges[l].start);
		     m <= grub_be_to_cpu16 (cover->ranges[l].end); m++)
		  subst (m);
	    }
	  else
	    printf ("Unsupported coverage specification: %d\n", covertype);
	}
    }
}

void
add_font (struct grub_font_info *font_info, FT_Face face, int nocut)
{
  struct gsub_header *gsub = NULL;
  FT_ULong gsub_len = 0;

  if (!FT_Load_Sfnt_Table (face, TTAG_GSUB, 0, NULL, &gsub_len))
    {
      gsub = xmalloc (gsub_len);
      if (FT_Load_Sfnt_Table (face, TTAG_GSUB, 0, (void *) gsub, &gsub_len))
	{
	  free (gsub);
	  gsub = NULL;
	  gsub_len = 0;
	}
    }
  if (gsub)
    {
      struct gsub_features *features 
	= (struct gsub_features *) (((grub_uint8_t *) gsub)
				    + grub_be_to_cpu16 (gsub->features_off));
      struct gsub_lookup_list *lookups
	= (struct gsub_lookup_list *) (((grub_uint8_t *) gsub)
				       + grub_be_to_cpu16 (gsub->lookups_off));
      int i;
      int nfeatures = grub_be_to_cpu16 (features->count);
      for (i = 0; i < nfeatures; i++)
	{
	  struct gsub_feature *feature = (struct gsub_feature *)
	    ((grub_uint8_t *) features
	     + grub_be_to_cpu16 (features->features[i].offset));
	  grub_uint32_t feattag
	    = grub_be_to_cpu32 (features->features[i].feature_tag);
	  if (feature->params)
	    printf ("WARNING: unsupported feature parameters: %x\n",
		    grub_be_to_cpu16 (feature->params));
	  switch (feattag)
	    {
	      /* Used for retrieving all possible variants. Useless in grub.  */
	    case FEATURE_AALT:
	      break;

	      /* FIXME: Add ligature support.  */
	    case FEATURE_LIGA:
	    case FEATURE_RLIG:
	      break;

	      /* Cursive form variants.  */
	    case FEATURE_FINA:
	    case FEATURE_INIT:
	    case FEATURE_MEDI:
	      process_cursive (feature, lookups, feattag);
	      break;

	    default:
	      {
		char str[5];
		int j;
		memcpy (str, &features->features[i].feature_tag,
			sizeof (features->features[i].feature_tag));
		str[4] = 0;
		for (j = 0; j < 4; j++)
		  if (!grub_isgraph (str[j]))
		    str[j] = '?';
		printf ("Unknown gsub feature 0x%x (%s)\n", feattag, str);
	      }
	    }
	}
    }

  if (font_info->num_range)
    {
      int i;
      grub_uint32_t j;

      for (i = 0; i < font_info->num_range; i++)
	for (j = font_info->ranges[i * 2]; j <= font_info->ranges[i * 2 + 1];
	     j++)
	  add_char (font_info, face, j, nocut);
    }
  else
    {
      grub_uint32_t char_code, glyph_index;

      for (char_code = FT_Get_First_Char (face, &glyph_index);
	   glyph_index;
	   char_code = FT_Get_Next_Char (face, char_code, &glyph_index))
	add_char (font_info, face, char_code, nocut);
    }
}

void
write_string_section (char *name, char *str, int* offset, FILE* file)
{
  grub_uint32_t leng, leng_be32;

  leng = strlen (str) + 1;
  leng_be32 = grub_cpu_to_be32 (leng);

  grub_util_write_image (name, 4, file);
  grub_util_write_image ((char *) &leng_be32, 4, file);
  grub_util_write_image (str, leng, file);

  *offset += 8 + leng;
}

void
write_be16_section (char *name, grub_uint16_t data, int* offset, FILE* file)
{
  grub_uint32_t leng;

  leng = grub_cpu_to_be32 (2);
  data = grub_cpu_to_be16 (data);
  grub_util_write_image (name, 4, file);
  grub_util_write_image ((char *) &leng, 4, file);
  grub_util_write_image ((char *) &data, 2, file);

  *offset += 10;
}

void
print_glyphs (struct grub_font_info *font_info)
{
  int num;
  struct grub_glyph_info *glyph;
  char line[512];

  for (glyph = font_info->glyphs_sorted, num = 0; num < font_info->num_glyphs;
       glyph++, num++)
    {
      int x, y, xmax, xmin, ymax, ymin;
      grub_uint8_t *bitmap, mask;

      printf ("\nGlyph #%d, U+%04x\n", num, glyph->char_code);
      printf ("Width %d, Height %d, X offset %d, Y offset %d, Device width %d\n",
	      glyph->width, glyph->height, glyph->x_ofs, glyph->y_ofs,
	      glyph->device_width);

      xmax = glyph->x_ofs + glyph->width;
      if (xmax < glyph->device_width)
	xmax = glyph->device_width;

      xmin = glyph->x_ofs;
      if (xmin > 0)
	xmin = 0;

      ymax = glyph->y_ofs + glyph->height;
      if (ymax < font_info->asce)
	ymax = font_info->asce;

      ymin = glyph->y_ofs;
      if (ymin > - font_info->desc)
	ymin = - font_info->desc;

      bitmap = glyph->bitmap;
      mask = 0x80;
      for (y = ymax - 1; y >= ymin; y--)
	{
	  int line_pos;

	  line_pos = 0;
	  for (x = xmin; x < xmax; x++)
	    {
	      if ((x >= glyph->x_ofs) &&
		  (x < glyph->x_ofs + glyph->width) &&
		  (y >= glyph->y_ofs) &&
		  (y < glyph->y_ofs + glyph->height))
		{
		  line[line_pos++] = (*bitmap & mask) ? '#' : '_';
		  mask >>= 1;
		  if (mask == 0)
		    {
		      mask = 0x80;
		      bitmap++;
		    }
		}
	      else if ((x >= 0) &&
		       (x < glyph->device_width) &&
		       (y >= - font_info->desc) &&
		       (y < font_info->asce))
		{
		  line[line_pos++] = ((x == 0) || (y == 0)) ? '+' : '.';
		}
	      else
		line[line_pos++] = '*';
	    }
	  line[line_pos] = 0;
	  printf ("%s\n", line);
	}
    }
}

void
write_font_ascii_bitmap (struct grub_font_info *font_info, char *output_file)
{
  FILE *file;
  struct grub_glyph_info *glyph;
  int num; 
  
  file = fopen (output_file, "wb");
  if (! file)
    grub_util_error ("Can\'t write to file %s.", output_file);

  int correct_size;
  for (glyph = font_info->glyphs_sorted, num = 0; num < font_info->num_glyphs;
       glyph++, num++)
    {
      correct_size = 1;
      if (glyph->width != 8 || glyph->height != 16)
      {
        /* printf ("Width or height from glyph U+%04x not supported, skipping.\n", glyph->char_code);  */
	correct_size = 0;
      }
      int row;
      for (row = 0; row < glyph->height; row++)
        {
	  if (correct_size)
	    fwrite (&glyph->bitmap[row], sizeof(glyph->bitmap[row]), 1, file);
	  else
	    fwrite (&correct_size, 1, 1, file);
        }
    }
    fclose (file);
}

void
write_font_width_spec (struct grub_font_info *font_info, char *output_file)
{
  FILE *file;
  struct grub_glyph_info *glyph;
  grub_uint8_t *out;

  out = xmalloc (8192);
  memset (out, 0, 8192);
  
  file = fopen (output_file, "wb");
  if (! file)
    grub_util_error ("Can\'t write to file %s.", output_file);

  for (glyph = font_info->glyphs_sorted;
       glyph < font_info->glyphs_sorted + font_info->num_glyphs; glyph++)
    if (glyph->width > 12)
      out[glyph->char_code >> 3] |= (1 << (glyph->char_code & 7));

  fwrite (out, 8192, 1, file);
  fclose (file);
  free (out);
}

void
write_font_pf2 (struct grub_font_info *font_info, char *output_file)
{
  FILE *file;
  grub_uint32_t leng, data;
  char style_name[20], *font_name;
  int offset;
  struct grub_glyph_info *cur;

  file = fopen (output_file, "wb");
  if (! file)
    grub_util_error ("can\'t write to file %s.", output_file);

  offset = 0;

  leng = grub_cpu_to_be32 (4);
  grub_util_write_image (FONT_FORMAT_SECTION_NAMES_FILE,
  			 sizeof(FONT_FORMAT_SECTION_NAMES_FILE) - 1, file);
  grub_util_write_image ((char *) &leng, 4, file);
  grub_util_write_image (FONT_FORMAT_PFF2_MAGIC, 4, file);
  offset += 12;

  if (! font_info->name)
    font_info->name = "Unknown";

  if (font_info->flags & GRUB_FONT_FLAG_BOLD)
    font_info->style |= FT_STYLE_FLAG_BOLD;

  style_name[0] = 0;
  if (font_info->style & FT_STYLE_FLAG_BOLD)
    strcpy (style_name, " Bold");

  if (font_info->style & FT_STYLE_FLAG_ITALIC)
    strcat (style_name, " Italic");

  if (! style_name[0])
    strcpy (style_name, " Regular");

  font_name = xasprintf ("%s %s %d", font_info->name, &style_name[1],
			 font_info->size);

  write_string_section (FONT_FORMAT_SECTION_NAMES_FONT_NAME,
  			font_name, &offset, file);
  write_string_section (FONT_FORMAT_SECTION_NAMES_FAMILY,
  			font_info->name, &offset, file);
  write_string_section (FONT_FORMAT_SECTION_NAMES_WEIGHT,
			(font_info->style & FT_STYLE_FLAG_BOLD) ?
			"bold" : "normal",
			&offset, file);
  write_string_section (FONT_FORMAT_SECTION_NAMES_SLAN,
			(font_info->style & FT_STYLE_FLAG_ITALIC) ?
			"italic" : "normal",
			&offset, file);

  write_be16_section (FONT_FORMAT_SECTION_NAMES_POINT_SIZE,
  		      font_info->size, &offset, file);
  write_be16_section (FONT_FORMAT_SECTION_NAMES_MAX_CHAR_WIDTH,
  		      font_info->max_width, &offset, file);
  write_be16_section (FONT_FORMAT_SECTION_NAMES_MAX_CHAR_HEIGHT,
  		      font_info->max_height, &offset, file);

  if (! font_info->desc)
    {
      if (font_info->min_y >= 0)
	font_info->desc = 1;
      else
	font_info->desc = - font_info->min_y;
    }

  if (! font_info->asce)
    {
      if (font_info->max_y <= 0)
	font_info->asce = 1;
      else
	font_info->asce = font_info->max_y;
    }

  write_be16_section (FONT_FORMAT_SECTION_NAMES_ASCENT,
  		      font_info->asce, &offset, file);
  write_be16_section (FONT_FORMAT_SECTION_NAMES_DESCENT,
  		      font_info->desc, &offset, file);

  if (font_verbosity > 0)
    {
      printf ("Font name: %s\n", font_name);
      printf ("Max width: %d\n", font_info->max_width);
      printf ("Max height: %d\n", font_info->max_height);
      printf ("Font ascent: %d\n", font_info->asce);
      printf ("Font descent: %d\n", font_info->desc);
    }

  if (font_verbosity > 0)
    printf ("Number of glyph: %d\n", font_info->num_glyphs);

  leng = grub_cpu_to_be32 (font_info->num_glyphs * 9);
  grub_util_write_image (FONT_FORMAT_SECTION_NAMES_CHAR_INDEX,
  			 sizeof(FONT_FORMAT_SECTION_NAMES_CHAR_INDEX) - 1,
			 file);
  grub_util_write_image ((char *) &leng, 4, file);
  offset += 8 + font_info->num_glyphs * 9 + 8;

  for (cur = font_info->glyphs_sorted;
       cur < font_info->glyphs_sorted + font_info->num_glyphs; cur++)
    {
      data = grub_cpu_to_be32 (cur->char_code);
      grub_util_write_image ((char *) &data, 4, file);
      data = 0;
      grub_util_write_image ((char *) &data, 1, file);
      data = grub_cpu_to_be32 (offset);
      grub_util_write_image ((char *) &data, 4, file);
      offset += 10 + cur->bitmap_size;
    }

  leng = 0xffffffff;
  grub_util_write_image (FONT_FORMAT_SECTION_NAMES_DATA,
  			 sizeof(FONT_FORMAT_SECTION_NAMES_DATA) - 1, file);
  grub_util_write_image ((char *) &leng, 4, file);

  for (cur = font_info->glyphs_sorted;
       cur < font_info->glyphs_sorted + font_info->num_glyphs; cur++)
    {
      data = grub_cpu_to_be16 (cur->width);
      grub_util_write_image ((char *) &data, 2, file);
      data = grub_cpu_to_be16 (cur->height);
      grub_util_write_image ((char *) &data, 2, file);
      data = grub_cpu_to_be16 (cur->x_ofs);
      grub_util_write_image ((char *) &data, 2, file);
      data = grub_cpu_to_be16 (cur->y_ofs);
      grub_util_write_image ((char *) &data, 2, file);
      data = grub_cpu_to_be16 (cur->device_width);
      grub_util_write_image ((char *) &data, 2, file);
      grub_util_write_image ((char *) &cur->bitmap[0], cur->bitmap_size, file);
    }

  fclose (file);
}

int
main (int argc, char *argv[])
{
  struct grub_font_info font_info;
  FT_Library ft_lib;
  int font_index = 0;
  int font_size = 0;
  char *output_file = NULL;
  enum file_formats file_format = PF2;

  memset (&font_info, 0, sizeof (font_info));

  set_program_name (argv[0]);

  grub_util_init_nls ();

  /* Check for options.  */
  while (1)
    {
      int c = getopt_long (argc, argv, "bao:n:i:s:d:r:hVv", options, 0);

      if (c == -1)
	break;
      else
	switch (c)
	  {
	  case 'b':
	    font_info.flags |= GRUB_FONT_FLAG_BOLD;
	    break;

	  case 0x100:
	    font_info.flags |= GRUB_FONT_FLAG_NOBITMAP;
	    break;

	  case 0x101:
	    font_info.flags |= GRUB_FONT_FLAG_NOHINTING;
	    break;

	  case 'a':
	    font_info.flags |= GRUB_FONT_FLAG_FORCEHINT;
	    break;

	  case 'o':
	    output_file = optarg;
	    break;

	  case 'n':
	    font_info.name = optarg;
	    break;

	  case 'i':
	    font_index = strtoul (optarg, NULL, 0);
	    break;

	  case 's':
	    font_size = strtoul (optarg, NULL, 0);
	    break;

	  case 'r':
	    {
	      char *p = optarg;

	      while (1)
		{
		  grub_uint32_t a, b;

		  a = strtoul (p, &p, 0);
		  if (*p != '-')
		    grub_util_error ("invalid font range");
		  b = strtoul (p + 1, &p, 0);
		  if ((font_info.num_range & (GRUB_FONT_RANGE_BLOCK - 1)) == 0)
		    font_info.ranges = xrealloc (font_info.ranges,
						 (font_info.num_range +
						  GRUB_FONT_RANGE_BLOCK) *
						 sizeof (grub_uint32_t) * 2);

		  font_info.ranges[font_info.num_range * 2] = a;
		  font_info.ranges[font_info.num_range * 2 + 1] = b;
		  font_info.num_range++;

		  if (*p)
		    {
		      if (*p != ',')
			grub_util_error ("invalid font range");
		      else
			p++;
		    }
		  else
		    break;
		}
	      break;
	    }

	  case 'd':
	    font_info.desc = strtoul (optarg, NULL, 0);
	    break;

	  case 'e':
	    font_info.asce = strtoul (optarg, NULL, 0);
	    break;

	  case 'h':
	    usage (0);
	    break;

	  case 'V':
	    printf ("%s (%s) %s\n", program_name, PACKAGE_NAME, PACKAGE_VERSION);
	    return 0;

	  case 'v':
	    font_verbosity++;
	    break;

	  case 0x102:
	     file_format = ASCII_BITMAPS;
	     break;

	  case 0x103:
	     file_format = WIDTH_SPEC;
	     break;

	  default:
	    usage (1);
	    break;
	  }
    }

  if (file_format == ASCII_BITMAPS && font_info.num_range > 0)
    {
      grub_util_error ("Option --ascii-bitmaps doesn't accept ranges (use ASCII).");
      return 1;
    }

  else if (file_format == ASCII_BITMAPS)
    {
      font_info.ranges = xrealloc (font_info.ranges,
				   GRUB_FONT_RANGE_BLOCK *
				   sizeof (grub_uint32_t) * 2);

      font_info.ranges[0] = (grub_uint32_t) 0x00;
      font_info.ranges[1] = (grub_uint32_t) 0x7f;
      font_info.num_range = 1;
    }

  if (! output_file)
    grub_util_error ("no output file is specified");

  if (FT_Init_FreeType (&ft_lib))
    grub_util_error ("FT_Init_FreeType fails");

  for (; optind < argc; optind++)
    {
      FT_Face ft_face;
      int size;
      FT_Error err;

      err = FT_New_Face (ft_lib, argv[optind], font_index, &ft_face);
      if (err)
	{
	  grub_printf ("can't open file %s, index %d: error %d", argv[optind],
		       font_index, err);
	  if (err > 0 && err < (signed) ARRAY_SIZE (ft_errmsgs))
	    printf (": %s\n", ft_errmsgs[err]);
	  else
	    printf ("\n");

	  continue;
	}

      if ((! font_info.name) && (ft_face->family_name))
	font_info.name = xstrdup (ft_face->family_name);

      size = font_size;
      if (! size)
	{
	  if ((ft_face->face_flags & FT_FACE_FLAG_SCALABLE) ||
	      (! ft_face->num_fixed_sizes))
	    size = GRUB_FONT_DEFAULT_SIZE;
	  else
	    size = ft_face->available_sizes[0].height;
	}

      font_info.style = ft_face->style_flags;
      font_info.size = size;

      if (FT_Set_Pixel_Sizes (ft_face, size, size))
	grub_util_error ("can't set %dx%d font size", size, size);
      add_font (&font_info, ft_face, file_format != PF2);
      FT_Done_Face (ft_face);
    }

  FT_Done_FreeType (ft_lib);

  {
    int counter[65537];
    struct grub_glyph_info *tmp, *cur;
    int i;

    memset (counter, 0, sizeof (counter));

    for (cur = font_info.glyphs_unsorted; cur; cur = cur->next)
      counter[(cur->char_code & 0xffff) + 1]++;
    for (i = 0; i < 0x10000; i++)
      counter[i+1] += counter[i];
    tmp = xmalloc (font_info.num_glyphs
		   * sizeof (tmp[0]));
    for (cur = font_info.glyphs_unsorted; cur; cur = cur->next)
      tmp[counter[(cur->char_code & 0xffff)]++] = *cur;

    memset (counter, 0, sizeof (counter));

    for (cur = tmp; cur < tmp + font_info.num_glyphs; cur++)
      counter[((cur->char_code & 0xffff0000) >> 16) + 1]++;
    for (i = 0; i < 0x10000; i++)
      counter[i+1] += counter[i];
    font_info.glyphs_sorted = xmalloc (font_info.num_glyphs
					* sizeof (font_info.glyphs_sorted[0]));
    for (cur = tmp; cur < tmp + font_info.num_glyphs; cur++)
      font_info.glyphs_sorted[counter[(cur->char_code & 0xffff0000) >> 16]++]
	= *cur;
    free (tmp);
  }

  switch (file_format)
    {
    case PF2:
      write_font_pf2 (&font_info, output_file);
      break;

    case ASCII_BITMAPS:
      write_font_ascii_bitmap (&font_info, output_file);
      break;

    case WIDTH_SPEC:
      write_font_width_spec (&font_info, output_file);
      break;
    }

  if (font_verbosity > 1)
    print_glyphs (&font_info);

  return 0;
}
