/*
 * Sylspell -- spell checking plug-in for Sylpheed
 * Copyright (C) 2012-2013 HAYASHI Kentaro
 *
 */

#ifndef __SYLSPELL_H__
#define __SYLSPELL_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#define SYLSPELL "sylspell"
#define SYLSPELL_JLP_KOUSEI "jlp_kousei"
#define SYLSPELLRC "sylspellrc"


#define PLUGIN_NAME N_("Sylspell - spell check plug-in for Sylpheed")
#define PLUGIN_DESC N_("spell check plug-in for Sylpheed")

struct _SylSpellOption {
  /* full path to ghostbiffrc*/
  gchar *rcpath;
  /* rcfile */
  GKeyFile *rcfile;

  gboolean startup_flg;
  gboolean send_flg;

  gboolean hunspell_flg;

  
  GtkWidget *startup;
  GtkWidget *send;

  GtkWidget *aspell;
  GtkWidget *hunspell;
  GtkWidget *jlp_kousei;

  GtkWidget *hunspell_hunspellcmd;
  GtkWidget *hunspell_hunspellcmd_path;

  /* Yahoo JAPAN Kousei */
  GtkWidget *jlp_kousei_appid;

  GtkWidget *jlp_kousei_group1;
  GtkWidget *jlp_kousei_group2;
  GtkWidget *jlp_kousei_group3;

  GtkWidget *jlp_kousei_misspell; /* mis spelling */
  GtkWidget *jlp_kousei_misuse; /* mis use */
  GtkWidget *jlp_kousei_note; /**/
  GtkWidget *jlp_kousei_unpleasant;
  GtkWidget *jlp_kousei_osdepends;
  GtkWidget *jlp_kousei_foreign;
  GtkWidget *jlp_kousei_noun;
  GtkWidget *jlp_kousei_jinmei;
  GtkWidget *jlp_kousei_ranuki;
  GtkWidget *jlp_kousei_phonetic;
  GtkWidget *jlp_kousei_nonstd;
  GtkWidget *jlp_kousei_usechar;
  GtkWidget *jlp_kousei_altusechar;
  GtkWidget *jlp_kousei_doublenegative;
  GtkWidget *jlp_kousei_particle;
  GtkWidget *jlp_kousei_verbosity;
  GtkWidget *jlp_kousei_abbr;

  GtkWidget *jlp_kousei_test;

  gboolean jlp_kousei_group1_flg;
  gboolean jlp_kousei_group2_flg;
  gboolean jlp_kousei_group3_flg;

  gboolean jlp_kousei_misspell_flg; /* mis spelling */
  gboolean jlp_kousei_misuse_flg; /* mis use */
  gboolean jlp_kousei_note_flg; /**/
  gboolean jlp_kousei_unpleasant_flg;
  gboolean jlp_kousei_osdepends_flg;
  gboolean jlp_kousei_foreign_flg;
  gboolean jlp_kousei_noun_flg;
  gboolean jlp_kousei_jinmei_flg;
  gboolean jlp_kousei_ranuki_flg;
  gboolean jlp_kousei_phonetic_flg;
  gboolean jlp_kousei_nonstd_flg;
  gboolean jlp_kousei_usechar_flg;
  gboolean jlp_kousei_altusechar_flg;
  gboolean jlp_kousei_doublenegative_flg;
  gboolean jlp_kousei_particle_flg;
  gboolean jlp_kousei_verbosity_flg;
  gboolean jlp_kousei_abbr_flg;

  GtkWidget *debug;
};

typedef struct _SylSpellOption SylSpellOption;

static void init_done_cb(GObject *obj, gpointer data);
static void app_exit_cb(GObject *obj, gpointer data);
static void app_force_exit_cb(GObject *obj, gpointer data);

static gchar *myprocmsg_get_message_file_path(MsgInfo *msginfo);
static void prefs_ok_cb(GtkWidget *widget, gpointer data);

static void exec_sylspell_cb(GObject *obj, FolderItem *item, const gchar *file, guint num);
static void exec_sylspell_menu_cb(void);
static void exec_sylspell_onoff_cb(void);
static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_jlp_kousei_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_snarl_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_growl_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey);

static void command_path_clicked(GtkWidget *widget, gpointer data);
static void inc_start_cb(GObject *obj, PrefsAccount *ac);
static void inc_finished_cb(GObject *obj, gint new_messages);

static void exec_sylspell_cb(GObject *obj, FolderItem *item, const gchar *file, guint num);
static void exec_sylspell_menu_cb(void);
static void exec_sylspell_onoff_cb(void);
static void compose_created_cb(GObject *obj, gpointer compose);
static void compose_destroy_cb(GObject *obj, gpointer compose);
static gboolean compose_send_cb(GObject *obj, gpointer compose,
                                gint compose_mode, gint send_mode,
                                const gchar *msg_file, GSList *to_list);
static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey);

void check_attachement_cb(GObject *obj, gpointer data);

void my_rmdir(gchar *path);
void my_rmdir_list(gchar *path);
static gint extract_attachment(AttachInfo *ainfo, gchar *dest, gchar *passwd);
void check_mailcontent_cb(GObject *obj, gpointer data);
static gboolean create_config_myframe (GtkWidget **app_align, GtkWidget **vbox_app, gchar *title);
static char *my_g_uri_escape_string(const char *unescaped,
                                    const char *reserved_chars_allowed,
                                    gboolean allow_utf8);


#define GET_RC_BOOLEAN(section, keyarg) g_key_file_get_boolean(g_opt.rcfile, section, keyarg, NULL)
#define SET_RC_BOOLEAN(section, keyarg,valarg) g_key_file_set_boolean(g_opt.rcfile, section, keyarg, valarg)
#define GET_RC_STRING(section, keyarg) g_key_file_get_string(g_opt.rcfile, section, keyarg, NULL)
#define SET_RC_STRING(section, keyarg,valarg) g_key_file_set_string(g_opt.rcfile, section, keyarg, valarg)

#define ALIGN_TOP 3
#define ALIGN_BOTTOM 3
#define ALIGN_LEFT 6
#define ALIGN_RIGHT 6
#define BOX_SPACE 6

#endif /* __SYLSPELL_H__ */
