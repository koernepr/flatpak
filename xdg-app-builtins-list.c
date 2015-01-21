#include "config.h"

#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "libgsystem.h"

#include "xdg-app-builtins.h"
#include "xdg-app-utils.h"

static gboolean opt_show_details;

static GOptionEntry options[] = {
  { "show-details", 0, 0, G_OPTION_ARG_NONE, &opt_show_details, "Show arches and branches", NULL },
  { NULL }
};

static gboolean
print_installed_refs (GFile *base, GCancellable *cancellable, GError **error)
{
  gboolean ret = FALSE;
  gs_unref_object GFileEnumerator *dir_enum = NULL;
  gs_unref_object GFileInfo *child_info = NULL;
  GError *temp_error = NULL;
  gs_unref_ptrarray GPtrArray *refs = NULL;
  int i;

  refs = g_ptr_array_new ();

  dir_enum = g_file_enumerate_children (base, G_FILE_ATTRIBUTE_STANDARD_NAME,
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        cancellable, error);
  if (!dir_enum)
    goto out;

  while ((child_info = g_file_enumerator_next_file (dir_enum, cancellable, &temp_error)))
    {
      gs_unref_object GFile *child = NULL;
      gs_unref_object GFileEnumerator *dir_enum2 = NULL;
      gs_unref_object GFileInfo *child_info2 = NULL;
      const char *name;

      name = g_file_info_get_name (child_info);

      child = g_file_get_child (base, name);
      g_clear_object (&dir_enum2);
      dir_enum2 = g_file_enumerate_children (child, G_FILE_ATTRIBUTE_STANDARD_NAME,
                                             G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                             cancellable, error);
      if (!dir_enum2)
        goto out;

      while ((child_info2 = g_file_enumerator_next_file (dir_enum2, cancellable, &temp_error)))
        {
          gs_unref_object GFile *child2 = NULL;
          gs_unref_object GFileEnumerator *dir_enum3 = NULL;
          gs_unref_object GFileInfo *child_info3 = NULL;
          const char *arch;

          arch = g_file_info_get_name (child_info2);
          if (strcmp (arch, "data") == 0)
            continue;

          child2 = g_file_get_child (child, arch);
          g_clear_object (&dir_enum3);
          dir_enum3 = g_file_enumerate_children (child2, G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                 G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                 cancellable, error);
          if (!dir_enum3)
            goto out;

          while ((child_info3 = g_file_enumerator_next_file (dir_enum3, cancellable, &temp_error)))
            {
              const char *branch;
              char *ref;

              branch = g_file_info_get_name (child_info3);

              if (opt_show_details)
                ref = g_strdup_printf ("%s/%s/%s", name, arch, branch);
              else
                ref = g_strdup (name);

              for (i = 0; i < refs->len; i++)
                {
                  int cmp;

                  cmp = strcmp (ref, g_ptr_array_index (refs, i));
                  if (cmp > 0)
                    continue;
                  else if (cmp < 0)
                    g_ptr_array_insert (refs, i, ref);
                  else
                    g_free (ref);
                  break;
                }
              if (i == refs->len)
                g_ptr_array_insert (refs, i, ref);

              g_clear_object (&child_info3);
            }

          if (temp_error != NULL)
            goto out;

          g_clear_object (&child_info2);
        }

      if (temp_error != NULL)
        goto out;

      g_clear_object (&child_info);
    }

  if (temp_error != NULL)
    goto out;

  for (i = 0; i < refs->len; i++)
    g_print ("%s\n", (char *)g_ptr_array_index (refs, i));

  ret = TRUE;

out:
  if (temp_error != NULL)
    g_propagate_error (error, temp_error);

  return ret;
}

gboolean
xdg_app_builtin_list_runtimes (int argc, char **argv, GCancellable *cancellable, GError **error)
{
  gboolean ret = FALSE;
  GOptionContext *context;
  gs_unref_object XdgAppDir *dir = NULL;
  gs_unref_object GFile *base = NULL;

  context = g_option_context_new (" - List installed runtimes");

  if (!xdg_app_option_context_parse (context, options, &argc, &argv, 0, &dir, cancellable, error))
    goto out;

  base = g_file_get_child (xdg_app_dir_get_path (dir), "runtime");
  if (!g_file_query_exists (base, cancellable))
    {
      ret = TRUE;
      goto out;
    }

  if (!print_installed_refs (base, cancellable, error))
    goto out;

  ret = TRUE;

 out:

  if (context)
    g_option_context_free (context);

  return ret;
}

gboolean
xdg_app_builtin_list_apps (int argc, char **argv, GCancellable *cancellable, GError **error)
{
  gboolean ret = FALSE;
  GOptionContext *context;
  gs_unref_object XdgAppDir *dir = NULL;
  gs_unref_object GFile *base = NULL;

  context = g_option_context_new (" - List installed applications");

  if (!xdg_app_option_context_parse (context, options, &argc, &argv, 0, &dir, cancellable, error))
    goto out;

  base = g_file_get_child (xdg_app_dir_get_path (dir), "app");
  if (!g_file_query_exists (base, cancellable))
    {
      ret = TRUE;
      goto out;
    }

  if (!print_installed_refs (base, cancellable, error))
    goto out;

  ret = TRUE;

 out:

  if (context)
    g_option_context_free (context);

  return ret;
}
