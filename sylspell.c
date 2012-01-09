/*
 * Check archive password Plug-in -- check your attachment archive is
 * password encrypted or not when you send mail.
 * Copyright (C) 2011 HAYASHI Kentaro <kenhys@gmail.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "defs.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/stat.h>

#include "sylmain.h"
#include "plugin.h"
#include "procmsg.h"
#include "procmime.h"
#include "utils.h"
#include "alertpanel.h"
#include "compose.h"

#include "tools-check-spelling.xpm"
#include "tools-check-spelling-off.xpm"

#include <glib/gi18n-lib.h>
#include <locale.h>

#include "sylspell.h"

static SylPluginInfo info = {
    N_(PLUGIN_NAME),
    "0.1.0",
    "HAYASHI Kentaro",
    N_(PLUGIN_DESC)
};



static gboolean g_enable = FALSE;

static GtkWidget *g_plugin_on = NULL;
static GtkWidget *g_plugin_off = NULL;
static GtkWidget *g_onoff_switch = NULL;
static GtkTooltips *g_tooltip = NULL;

static HANDLE g_hdll = NULL;

static SylSpellOption g_opt;

static gchar* g_copyright = N_("SylSpell is distributed under GPL license.\n"
"\n"
"Copyright (C) 2012 HAYASHI Kentaro <kenhys@gmail.com>"
"\n"
"sylspell contains following resource as statusbar icon.\n"
"\n\n"
"Silk icon set 1.3: Copyright (C) Mark James\n"
"Licensed under a Creative Commons Attribution 2.5 License.\n"
"http://www.famfamfam.com/lab/icons/silk/\n"
                               "\n"
                               );

void plugin_load(void)
{
  debug_print("[PLUGIN] initializing sylspell plug-in\n");

  syl_init_gettext(SYLSPELL, "lib/locale");

  debug_print(gettext(PLUGIN_NAME));
  debug_print(dgettext(SYLSPELL, PLUGIN_DESC));

  info.name = g_strdup(_(PLUGIN_NAME));
  info.description = g_strdup(_(PLUGIN_DESC));
  
  syl_plugin_add_menuitem("/Tools", NULL, NULL, NULL);
  syl_plugin_add_menuitem("/Tools", _("SylSpell Settings [sylspell]"), exec_sylspell_menu_cb, NULL);

  syl_plugin_signal_connect("compose-created", G_CALLBACK(compose_created_cb), NULL);

  syl_plugin_signal_connect("compose-destroy", G_CALLBACK(compose_destroy_cb), NULL);

  syl_plugin_signal_connect("compose-send", G_CALLBACK(compose_send_cb), NULL);

  GtkWidget *mainwin = syl_plugin_main_window_get();
  GtkWidget *statusbar = syl_plugin_main_window_get_statusbar();
  GtkWidget *plugin_box = gtk_hbox_new(FALSE, 0);

  GdkPixbuf* on_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)tools_check_spelling);
  g_plugin_on=gtk_image_new_from_pixbuf(on_pixbuf);
    
  GdkPixbuf* off_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)tools_check_spelling_off);
  g_plugin_off=gtk_image_new_from_pixbuf(off_pixbuf);

  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_on, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_off, FALSE, FALSE, 0);
    
  g_tooltip = gtk_tooltips_new();
    
  g_onoff_switch = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(g_onoff_switch), GTK_RELIEF_NONE);
  GTK_WIDGET_UNSET_FLAGS(g_onoff_switch, GTK_CAN_FOCUS);
  gtk_widget_set_size_request(g_onoff_switch, 20, 20);

  gtk_container_add(GTK_CONTAINER(g_onoff_switch), plugin_box);
  g_signal_connect(G_OBJECT(g_onoff_switch), "clicked",
                   G_CALLBACK(exec_sylspell_onoff_cb), mainwin);
  gtk_box_pack_start(GTK_BOX(statusbar), g_onoff_switch, FALSE, FALSE, 0);

  gtk_widget_show_all(g_onoff_switch);

  gtk_widget_hide(g_plugin_on);
  gtk_widget_show(g_plugin_off);
  gtk_tooltips_set_tip
    (g_tooltip, g_onoff_switch, _("SylSpell is disabled."),
     NULL);

  g_opt.startup_flg = FALSE;
    
  gchar *rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, SYLSPELLRC, NULL);
  g_opt.rcfile = g_key_file_new();
  g_opt.rcpath = g_strdup(rcpath);
  if (g_key_file_load_from_file(g_opt.rcfile, rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    g_opt.startup_flg=GET_RC_BOOLEAN(SYLSPELL, "startup");

    if (g_opt.startup_flg != FALSE){
      g_enable=TRUE;
      gtk_widget_hide(g_plugin_off);
      gtk_widget_show(g_plugin_on);
      gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                           _("SylSpell is enabled. Click the icon to disable plugin."),
                           NULL);

#if 0
      if (g_hdll){
        gtk_widget_hide(g_plugin_off);
        gtk_widget_show(g_plugin_on);
        gtk_tooltips_set_tip
          (g_tooltip, g_onoff_switch,
           _("SylSpell is enabled."),
           NULL);
      }else {
        gtk_widget_hide(g_plugin_on);
        gtk_widget_show(g_plugin_off);
        gtk_tooltips_set_tip
          (g_tooltip, g_onoff_switch, _("SylSpell is disabled."), NULL);
      }
#endif


    }
        
    g_opt.send_flg=GET_RC_BOOLEAN(SYLSPELL, "send");
      
    g_free(rcpath);
  }

  debug_print("[PLUGIN] sylspellrc startup:%d\n", g_opt.startup_flg);
  debug_print("[PLUGIN] sylspell_tool plug-in loading done.\n");

}

void plugin_unload(void)
{
  debug_print("sylspell_tool plug-in unloaded.\n");
  if (g_hdll!=NULL){
      FreeLibrary(g_hdll);
  }
  /* NOTE: in older GTK version,
     you cant use GIO, so remove tempolary file listing by myself. */
  gchar* path = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
                            PLUGIN_DIR,G_DIR_SEPARATOR_S, SYLSPELL, NULL);
  my_rmdir(path);
}

void my_rmdir(gchar *path)
{
#if 0
  g_opt.rmlist = NULL;
  my_rmdir_list(path);
  guint n = 0;
  for (n = 0;n< g_list_length(g_opt.rmlist); n++){
    gchar *path = (gchar*)g_list_nth_data(g_opt.rmlist, n);
    debug_print("list[%d]:%s\n", n, path);
    if (g_file_test(path, G_FILE_TEST_IS_DIR)!=FALSE){
      g_rmdir(path);
    } else {
      g_remove(path);
    }
  }
  g_rmdir(path);
#endif
}

void my_rmdir_list(gchar *dpath)
{
  GDir *g_dir = g_dir_open(dpath, 0, NULL);
  gchar *path = NULL;
#if 0
  while ((path = (gchar*)g_dir_read_name(g_dir))!=NULL){
    debug_print("[PLUGIN] path %s\n", path);
    gchar* fpath = g_strconcat(dpath, G_DIR_SEPARATOR_S, path, NULL);
    if (g_file_test(fpath, G_FILE_TEST_IS_DIR)!=FALSE){
#if DEBUG
      debug_print("[PLUGIN] remove dir %s\n", fpath);
#endif
      my_rmdir_list(fpath);
      g_opt.rmlist = g_list_append(g_opt.rmlist, fpath);
    }else {
#if DEBUG
      debug_print("[PLUGIN] remove %s\n", fpath);
#endif
      g_opt.rmlist = g_list_append(g_opt.rmlist, fpath);
    }
  }
#endif
  g_dir_close(g_dir);
}


SylPluginInfo *plugin_info(void)
{
  return &info;
}

gint plugin_interface_version(void)
{
#if RELEASE_3_1
    /* emulate sylpheed 3.1.0 not svn HEAD */
    return 0x0107;
#else
    /* sylpheed 3.2.0 or later. */
    return 0x0108;
    /*return SYL_PLUGIN_INTERFACE_VERSION;*/
#endif
}

#define STORE_TOGGLE_BUTTON(lbl, btn)                                    \
    g_opt.jlp_kousei_##btn##_flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.jlp_kousei_##btn)); \
  SET_RC_BOOLEAN(SYLSPELL_JLP_KOUSEI, lbl, g_opt.jlp_kousei_##btn##_flg); \
  debug_print("use %s:%s\n", lbl, g_opt.jlp_kousei_##btn##_flg ? "TRUE" : "FALSE");

static void prefs_ok_cb(GtkWidget *widget, gpointer data)
{

  g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);

  g_opt.startup_flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.startup));
  SET_RC_BOOLEAN(SYLSPELL, "startup", g_opt.startup_flg);
  debug_print("startup:%s\n", g_opt.startup_flg ? "TRUE" : "FALSE");

  gchar *appid = gtk_entry_get_text(GTK_ENTRY(g_opt.jlp_kousei_appid));
  if (appid != NULL) {
    g_key_file_set_string(g_opt.rcfile, SYLSPELL_JLP_KOUSEI, "appid", appid);
  }
  
  STORE_TOGGLE_BUTTON("group1", group1);
  STORE_TOGGLE_BUTTON("group2", group2);
  STORE_TOGGLE_BUTTON("group3", group3);

  STORE_TOGGLE_BUTTON("misspell", misspell);
  STORE_TOGGLE_BUTTON("misuse", misuse);
  STORE_TOGGLE_BUTTON("note", note);
  STORE_TOGGLE_BUTTON("unpleasant", unpleasant);
  STORE_TOGGLE_BUTTON("osdepends", osdepends);
  STORE_TOGGLE_BUTTON("foreign", foreign);
  STORE_TOGGLE_BUTTON("noun", noun);
  STORE_TOGGLE_BUTTON("jinmei", jinmei);
  STORE_TOGGLE_BUTTON("ranuki", ranuki);
  STORE_TOGGLE_BUTTON("phonetic", phonetic);
  STORE_TOGGLE_BUTTON("nonstd", nonstd);
  STORE_TOGGLE_BUTTON("usechar", usechar);
  STORE_TOGGLE_BUTTON("altusechar", altusechar);
  STORE_TOGGLE_BUTTON("doublenegative", doublenegative);
  STORE_TOGGLE_BUTTON("particle", particle);
  STORE_TOGGLE_BUTTON("verbosity", verbosity);
  STORE_TOGGLE_BUTTON("abbr", abbr);


  /**/
  gsize sz;
  gchar *buf=g_key_file_to_data(g_opt.rcfile, &sz, NULL);
  g_file_set_contents(g_opt.rcpath, buf, sz, NULL);

  gtk_widget_destroy(GTK_WIDGET(data));
}

static void prefs_cancel_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}


static void exec_sylspell_menu_cb(void)
{
  debug_print("[PLUGIN] exec_sylspell_menu_cb is called.\n");

  /* show modal dialog */
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *confirm_area;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  gtk_window_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
  gtk_widget_realize(window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  /* notebook */ 
  GtkWidget *notebook = gtk_notebook_new();
  /* main tab */
  create_config_main_page(notebook, g_opt.rcfile);
  /* Yahoo! JAPAN Kousei */
  create_config_jlp_kousei_page(notebook, g_opt.rcfile);
  /* about, copyright tab */
  create_config_about_page(notebook, g_opt.rcfile);

  gtk_widget_show(notebook);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

  confirm_area = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(confirm_area), 6);


  ok_btn = gtk_button_new_from_stock(GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(ok_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), ok_btn, FALSE, FALSE, 0);
  gtk_widget_show(ok_btn);

  cancel_btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  GTK_WIDGET_SET_FLAGS(cancel_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), cancel_btn, FALSE, FALSE, 0);
  gtk_widget_show(cancel_btn);

  gtk_widget_show(confirm_area);
	
  gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_grab_default(ok_btn);

  gtk_window_set_title(GTK_WINDOW(window), _("Sylspell Settings"));

  g_signal_connect(G_OBJECT(ok_btn), "clicked",
                   G_CALLBACK(prefs_ok_cb), window);
  g_signal_connect(G_OBJECT(cancel_btn), "clicked",
                   G_CALLBACK(prefs_cancel_cb), window);
  gtk_widget_show(window);

}

static void exec_sylspell_onoff_cb(void)
{

  if (g_enable != TRUE){
      syl_plugin_alertpanel_message(_("SylSpell"), _("SylSpell plugin is enabled."), ALERT_NOTICE);
      g_enable=TRUE;
      gtk_widget_hide(g_plugin_off);
      gtk_widget_show(g_plugin_on);
      gtk_tooltips_set_tip
        (g_tooltip, g_onoff_switch,
         _("Sylspell is enabled. Click the icon to disable plugin."),
         NULL);
  }else{
    syl_plugin_alertpanel_message(_("SylSpell"), _("SylSpell plugin is disabled."), ALERT_NOTICE);
    g_enable=FALSE;
    gtk_widget_hide(g_plugin_on);
    gtk_widget_show(g_plugin_off);
    gtk_tooltips_set_tip
      (g_tooltip, g_onoff_switch,
       _("SylSpell is disabled. Click the icon to enable plugin."),
       NULL);
  }
}

void compose_created_cb(GObject *obj, gpointer data)
{
  Compose *compose = (Compose*)data;


  /* add spell check button for testing. */
  GtkWidget *toolbar = compose->toolbar;
  GtkWidget *icon = gtk_image_new_from_stock(GTK_STOCK_SPELL_CHECK, GTK_ICON_SIZE_LARGE_TOOLBAR);  
  GtkToolItem *toolitem = gtk_tool_button_new(icon, _("spell check"));
  GtkTooltips *tooltips = gtk_tooltips_new();
  gtk_tool_item_set_tooltip(toolitem, tooltips, _("spell check content of your mail"), "");
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
  
  g_signal_connect(G_OBJECT(toolitem), "clicked",
                   G_CALLBACK(check_mailcontent_cb), compose);
#if 0
  g_signal_connect(G_OBJECT(GTK_BIN(toolitem)->child),
                   "button_press_event",
                   G_CALLBACK(check_mailcontent_cb), compose);
#endif
  gtk_widget_show_all(toolbar);
}

void compose_destroy_cb(GObject *obj, gpointer compose)
{
  debug_print("[PLUGIN] compose_destroy_cb is called.\n");
  /**/
}

void check_mailcontent_cb(GObject *obj, gpointer data)
{
  Compose *compose = (Compose*)data;

  gchar *appid = g_key_file_get_string(g_opt.rcfile, SYLSPELL_JLP_KOUSEI, "appid", NULL);
  if (appid != NULL) {

    gchar *com = NULL;

    GtkTextView *text = GTK_TEXT_VIEW(compose->text);
	GtkTextBuffer *buffer;
    GtkTextIter tsiter, teiter;
	buffer = gtk_text_view_get_buffer(text);
    gtk_text_buffer_get_bounds(buffer, &tsiter, &teiter);
    gchar *buf = gtk_text_buffer_get_text(buffer,&tsiter, &teiter, FALSE);
    
    com = g_strdup_printf("curl --verbose --data 'appid=%s' --data 'sentence=%s' %s",
                          appid, encode_uri(buf), "http://jlp.yahooapis.jp/KouseiService/V1/kousei");
#if 0
    com = g_strdup_printf("curl --data 'appid=%s' --data 'sentence=%s' %s",
                          appid, encode_url(buf), "http://jlp.yahooapis.jp/KouseiService/V1/kousei");
#endif
    debug_print("UTF-8:%s\n", buf);
    gchar *uri = g_filename_to_uri(buf, NULL, NULL);
    if (uri) {
      debug_print("UTF-8:%s\n", uri);
    }
    debug_print("curl command:%s\n", com);

#if 0
    gchar *result = get_command_output(com);
    debug_print("result:%s\n", result);
#endif  
  }
}

static gboolean compose_send_cb(GObject *obj, gpointer data,
                                gint compose_mode, gint send_mode,
                                const gchar *msg_file, GSList *to_list)
{

  Compose *compose = (Compose*)data;
  debug_print("[PLUGIN] compose_send_cb is called.\n");

  debug_print("Compose* compose:%p\n", compose);
  debug_print("gpointer compose:%p\n", data);

  GtkTreeModel *model = GTK_TREE_MODEL(compose->attach_store);
  GtkTreeIter iter;
  AttachInfo *ainfo;
  gboolean valid;

  debug_print("model:%p\n", model);
  gint nblank;
  gint npasswd;
  
  /* create working directory for unzip */
  gchar* path = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
                            PLUGIN_DIR,G_DIR_SEPARATOR_S, "sylspell", NULL);
  g_mkdir_with_parents(path, 0);
  
  gint npasstotal = 0;
  gint npassok = 0;
  gboolean bpasswd = FALSE;

  debug_print("text:%p\n", compose->text);
  GtkTextView *text = GTK_TEXT_VIEW(compose->text);
  if (text==NULL){
    debug_print("text is NULL\n");
    return TRUE;
  }else{
    debug_print("text:%p\n", text);
  }
  GtkTextBuffer *buffer;
  buffer = gtk_text_view_get_buffer(text);
  GtkTextIter tsiter, teiter;
  if (buffer == NULL){
    debug_print("buffer is NULL\n");
    return TRUE;
  }else{
    debug_print("buffer:%p\n", buffer);
  }
  gtk_text_buffer_get_bounds(buffer, &tsiter, &teiter);
  gchar *pwtext = gtk_text_buffer_get_text(buffer, &tsiter, &teiter, TRUE);
  GScanner *gscan = g_scanner_new(NULL);
  gscan->config->scan_identifier_1char=TRUE;

  if (pwtext==NULL){
    debug_print("pwtext is NULL\n");
  }else{
    g_scanner_input_text(gscan, pwtext, strlen(pwtext));
    debug_print("pwtext:%p\n", pwtext);
  }

  debug_print("scan loop\n");
  GTokenValue gvalue;
  GList *pwlist = NULL;
  int index=0;
  while( g_scanner_eof(gscan) != TRUE){
    GTokenType gtoken = g_scanner_get_next_token (gscan);
    switch (gtoken){
    case G_TOKEN_CHAR:
      gvalue = g_scanner_cur_value(gscan);
      g_print("char:%s\n", gvalue.v_identifier);
      break;
    case G_TOKEN_IDENTIFIER:
      gvalue = g_scanner_cur_value(gscan);
      for (index = 0; index<strlen(gvalue.v_identifier);index++){
        if (index == strlen(gvalue.v_identifier)-1 &&
            gvalue.v_identifier[index] & 0xffff0000){
          gvalue.v_identifier[index]= '\0';
        }else{
          g_print("char :%0x08\n", gvalue.v_identifier[index]);
        }
      }
      g_print("identifier:%s\n", gvalue.v_identifier);
      pwlist = g_list_append(pwlist, g_strdup(gvalue.v_identifier));
      break;
    default:
      break;
    }
  }
  
  /* get password candidate from text */
  gchar *msg=NULL;

  gboolean bcancel = TRUE;
  g_print("compose_send_cb:%s\n", bcancel ? "TRUE" : "FALSE");
  return bcancel;
}

static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey)
{
  debug_print("[PLUGIN] create_config_main_page\n");
  if (notebook == NULL){
    return NULL;
  }
  /* startup */
  if (pkey!=NULL){
  }
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *startup_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *startup_frm = gtk_frame_new(_("Startup Option"));
  GtkWidget *startup_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  g_opt.startup = gtk_check_button_new_with_label(_("Enable plugin on startup."));
  gtk_box_pack_start(GTK_BOX(vbox), g_opt.startup, FALSE, FALSE, 0);

  g_opt.startup = gtk_check_button_new_with_label(_("Enable loading dictionary on startup."));
  gtk_box_pack_start(GTK_BOX(vbox), g_opt.startup, FALSE, FALSE, 0);

  g_opt.send = gtk_check_button_new_with_label(_("Enable spell checking before sending mail."));
  gtk_box_pack_start(GTK_BOX(vbox), g_opt.send, FALSE, FALSE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("General"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);

  gboolean flg=g_key_file_get_boolean(pkey, SYLSPELL, "startup", NULL);
  debug_print("startup:%s\n", flg ? "TRUE" : "FALSE");
  if (flg!=FALSE){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.startup), TRUE);
  }

  /* Application */
  GtkWidget *app_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *app_frm = gtk_frame_new(_("Application/Service"));
  GtkWidget *app_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  g_opt.hunspell = gtk_radio_button_new_with_label(NULL, _("Hunspell"));
  g_opt.aspell = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (g_opt.hunspell), _("Aspell"));
  g_opt.jlp_kousei = gtk_radio_button_new_with_label(NULL, _("Yahoo! JAPAN Kousei"));

  GtkWidget *vbox_app = gtk_vbox_new(FALSE, BOX_SPACE);
  gtk_box_pack_start(GTK_BOX(vbox_app), g_opt.hunspell, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_app), g_opt.aspell, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_app), g_opt.jlp_kousei, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(app_frm_align), vbox_app);
  gtk_container_add(GTK_CONTAINER(app_frm), app_frm_align);
  gtk_container_add(GTK_CONTAINER(app_align), app_frm);

  flg=g_key_file_get_boolean(pkey, SYLSPELL, "send", NULL);
  debug_print("send:%s\n", flg ? "TRUE" : "FALSE");
  if (flg!=FALSE){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.send), TRUE);
  }

#if 0
  flg=g_key_file_get_boolean(pkey, SYLSPELL, "passwd", NULL);
  debug_print("startup:%s\n", flg ? "TRUE" : "FALSE");
  if (flg!=FALSE){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.chk_passwd), TRUE);
  }
#endif
  
  return NULL;
}

#define LOAD_TOGGLE_BUTTON(lbl, btn)                                    \
  g_opt.jlp_kousei_##btn##_flg = GET_RC_BOOLEAN(SYLSPELL_JLP_KOUSEI, lbl); \
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.jlp_kousei_##btn), g_opt.jlp_kousei_##btn##_flg); \
  debug_print("use %s:%s\n", lbl, g_opt.jlp_kousei_##btn##_flg ? "TRUE" : "FALSE");
/**
 */
static GtkWidget *create_config_jlp_kousei_page(GtkWidget *notebook, GKeyFile *pkey)
{
  debug_print("[PLUGIN] create_config_jlp_kousei_page\n");
  if (notebook == NULL){
    return NULL;
  }
  /* startup */
  if (pkey!=NULL){
  }
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  /* Application ID */
  GtkWidget *app_align = NULL;
  GtkWidget *vbox_app = NULL;
  create_config_myframe(&app_align, &vbox_app, _("Yahoo JAPAN Application ID"));
  g_opt.jlp_kousei_appid = gtk_entry_new();
  GtkWidget *appid_box = gtk_hbox_new(FALSE, BOX_SPACE);
  GtkWidget *appid_lbl = gtk_label_new(_("Application ID:"));
  gtk_box_pack_start(GTK_BOX(appid_box), appid_lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(appid_box), g_opt.jlp_kousei_appid, TRUE, TRUE, 0);
  
  gtk_box_pack_start(GTK_BOX(vbox_app), appid_box, FALSE, FALSE, 0);

  /* Filter Group */
  GtkWidget *grp_align = NULL;
  GtkWidget *vbox_grp = NULL;
  create_config_myframe(&grp_align, &vbox_grp, _("Filter group"));
  g_opt.jlp_kousei_group1 = gtk_check_button_new_with_label(_("Point out misspelling, not proper expression."));
  g_opt.jlp_kousei_group2 = gtk_check_button_new_with_label(_("Point out using simple English."));
  g_opt.jlp_kousei_group3 = gtk_check_button_new_with_label(_("Point out how to improve your sentenses."));

  gtk_box_pack_start(GTK_BOX(vbox_grp), g_opt.jlp_kousei_group1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_grp), g_opt.jlp_kousei_group2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_grp), g_opt.jlp_kousei_group3, FALSE, FALSE, 0);
  
  /* NO filter */
  GtkWidget *nofil_align = NULL;
  GtkWidget *vbox_nofil = NULL;
  create_config_myframe(&nofil_align, &vbox_nofil, _("No filter"));
  g_opt.jlp_kousei_misspell = gtk_check_button_new_with_label(_("Ignore misspelling."));
  g_opt.jlp_kousei_misuse = gtk_check_button_new_with_label(_("Ignore misuse word."));
  g_opt.jlp_kousei_note = gtk_check_button_new_with_label(_("Ignore attention needed words."));
  g_opt.jlp_kousei_unpleasant = gtk_check_button_new_with_label(_("Ignore unpleasant words."));
  g_opt.jlp_kousei_osdepends = gtk_check_button_new_with_label(_("Ignore OS depends words."));
  g_opt.jlp_kousei_foreign = gtk_check_button_new_with_label(_("Ignore a foreign place name."));
  g_opt.jlp_kousei_noun = gtk_check_button_new_with_label(_("Ignore a proper noun."));
  g_opt.jlp_kousei_jinmei = gtk_check_button_new_with_label(_("Ignore Jinmei words."));
  g_opt.jlp_kousei_ranuki = gtk_check_button_new_with_label(_("Ignore RANUKI words."));
  g_opt.jlp_kousei_phonetic = gtk_check_button_new_with_label(_("Ignore a phonetic equivalent."));
  g_opt.jlp_kousei_nonstd = gtk_check_button_new_with_label(_("Ignore non starndard words."));
  g_opt.jlp_kousei_usechar = gtk_check_button_new_with_label(_("Ignore used characters."));
  g_opt.jlp_kousei_altusechar = gtk_check_button_new_with_label(_("Ignore alternative used characters."));
  g_opt.jlp_kousei_doublenegative = gtk_check_button_new_with_label(_("Ignore a double negative expression."));
  g_opt.jlp_kousei_particle = gtk_check_button_new_with_label(_("Ignore a particle."));
  g_opt.jlp_kousei_verbosity = gtk_check_button_new_with_label(_("Ignore verbosity words."));
  g_opt.jlp_kousei_abbr = gtk_check_button_new_with_label(_("Ignore abbr words."));

  GtkWidget *nofil_tbl = gtk_table_new(9, 2, TRUE);
  gtk_box_pack_start(GTK_BOX(vbox_nofil), nofil_tbl, TRUE, TRUE, 0);

  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_misspell, 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_misuse, 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_note, 0, 1, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_unpleasant, 0, 1, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_osdepends, 0, 1, 4, 5);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_foreign, 0, 1, 5, 6);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_noun, 0, 1, 6, 7);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_jinmei, 0, 1, 7, 8);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_ranuki, 0, 1, 8, 9);

  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_phonetic, 1, 2, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_nonstd, 1, 2, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_usechar, 1, 2, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_altusechar, 1, 2, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_doublenegative, 1, 2, 4, 5);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_particle, 1, 2, 5, 6);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_verbosity, 1, 2, 6, 7);
  gtk_table_attach_defaults(GTK_TABLE(nofil_tbl), g_opt.jlp_kousei_abbr, 1, 2, 7, 8);

#if 0
  flg=g_key_file_get_boolean(pkey, SYLSPELL, "send", NULL);
  debug_print("send:%s\n", flg ? "TRUE" : "FALSE");
  if (flg!=FALSE){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.send), TRUE);
  }

  flg=g_key_file_get_boolean(pkey, SYLSPELL, "passwd", NULL);
  debug_print("startup:%s\n", flg ? "TRUE" : "FALSE");
  if (flg!=FALSE){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.chk_passwd), TRUE);
  }
#endif

  gtk_box_pack_start(GTK_BOX(vbox), app_align, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), grp_align, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), nofil_align, FALSE, FALSE, 0);
  

  gchar *appid = g_key_file_get_string(g_opt.rcfile, SYLSPELL_JLP_KOUSEI, "appid", NULL);
  if (appid != NULL) {
    gtk_entry_set_text(GTK_ENTRY(g_opt.jlp_kousei_appid), appid);
  }
  
  LOAD_TOGGLE_BUTTON("group1", group1);
  LOAD_TOGGLE_BUTTON("group2", group2);
  LOAD_TOGGLE_BUTTON("group3", group3);

  LOAD_TOGGLE_BUTTON("misspell", misspell);
  LOAD_TOGGLE_BUTTON("misuse", misuse);
  LOAD_TOGGLE_BUTTON("note", note);
  LOAD_TOGGLE_BUTTON("unpleasant", unpleasant);
  LOAD_TOGGLE_BUTTON("osdepends", osdepends);
  LOAD_TOGGLE_BUTTON("foreign", foreign);
  LOAD_TOGGLE_BUTTON("noun", noun);
  LOAD_TOGGLE_BUTTON("jinmei", jinmei);
  LOAD_TOGGLE_BUTTON("ranuki", ranuki);
  LOAD_TOGGLE_BUTTON("phonetic", phonetic);
  LOAD_TOGGLE_BUTTON("nonstd", nonstd);
  LOAD_TOGGLE_BUTTON("usechar", usechar);
  LOAD_TOGGLE_BUTTON("altusechar", altusechar);
  LOAD_TOGGLE_BUTTON("doublenegative", doublenegative);
  LOAD_TOGGLE_BUTTON("particle", particle);
  LOAD_TOGGLE_BUTTON("verbosity", verbosity);
  LOAD_TOGGLE_BUTTON("abbr", abbr);
      
  GtkWidget *general_lbl = gtk_label_new(_("Yahoo! JAPAN Kousei"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);
 return NULL;
}

static gboolean create_config_myframe (GtkWidget **app_align, GtkWidget **vbox_app, gchar *title)
{
  *app_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(*app_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *app_frm = gtk_frame_new(title);
  GtkWidget *app_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  *vbox_app = gtk_vbox_new(FALSE, BOX_SPACE);

  gtk_container_add(GTK_CONTAINER(app_frm_align), *vbox_app);
  gtk_container_add(GTK_CONTAINER(app_frm), app_frm_align);
  gtk_container_add(GTK_CONTAINER(*app_align), app_frm);
}

/* about, copyright tab */
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey)
{
  debug_print("create_config_about_page\n");
  if (notebook == NULL){
    return NULL;
  }
  GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 6);

  GtkWidget *misc = gtk_label_new("SylSpell");
  gtk_box_pack_start(GTK_BOX(vbox), misc, FALSE, TRUE, 6);

  misc = gtk_label_new(PLUGIN_DESC);
  gtk_box_pack_start(GTK_BOX(vbox), misc, FALSE, TRUE, 6);

  /* copyright */
  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);

  GtkTextBuffer *tbuffer = gtk_text_buffer_new(NULL);
  gtk_text_buffer_set_text(tbuffer, _(g_copyright), strlen(g_copyright));
  GtkWidget *tview = gtk_text_view_new_with_buffer(tbuffer);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tview), FALSE);
  gtk_container_add(GTK_CONTAINER(scrolled), tview);
    
  gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 6);
    
  /**/
  GtkWidget *general_lbl = gtk_label_new(_("About"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, general_lbl);
  gtk_widget_show_all(notebook);
  return NULL;
}

GtkWidget *g_popup = NULL;
gint g_popx, g_popy;
GtkWidget *g_label =NULL;

static gboolean textview_motion_notify(GtkWidget *widget,
                                       GdkEventMotion *event,
                                       GtkTextView *textview)
{
	gint x, y;
	GtkTextIter iter, cur,start, end;
 
	if (gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT) != event->window)
		return FALSE;
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget),
					      GTK_TEXT_WINDOW_WIDGET,
					      event->x, event->y, &x, &y);

    if (g_popx != -1 && g_popy != -1){
        if (x != g_popx || y != g_popy){
            gtk_widget_hide(g_popup);
        }
    }
    
#if 0
    debug_print("[PLUGIN] event x:%d event y:%d x:%d y:%d\n\n",event->x, event->y, x, y);
#endif

    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, x, y);
    gtk_text_iter_forward_word_end(&iter);
    end = iter;

    gtk_text_iter_backward_word_start(&iter);
    start = iter;
    
    gchar *text = gtk_text_iter_get_text(&start, &end);
    debug_print("motion text:%s\n", text);
    if (g_popup!=NULL){
        gtk_label_set_text(GTK_LABEL(g_label), text);
        gtk_widget_show_all(g_popup);
        g_popx = x;
        g_popy = y;
    }else {
#if 0
        g_popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_decorated(g_popup, FALSE);
#else
        g_popup = gtk_window_new(GTK_WINDOW_POPUP);
        gtk_window_set_position(GTK_WINDOW(g_popup), GTK_WIN_POS_MOUSE);
        gint px, py;
        gtk_window_get_position(GTK_WINDOW(g_popup), &px, &py);
        py-= 30;
        gtk_window_move(GTK_WINDOW(g_popup), px, py);
#endif
        g_label = gtk_label_new(text);
        gtk_container_add(GTK_CONTAINER(g_popup), g_label);
        gtk_widget_show_all(g_popup);
        g_popx = x;
        g_popy = y;
    }
#if 0
    textview_get_cursor(textview, GTK_TEXT_VIEW(widget), x, y);
#endif
    gdk_window_get_pointer(widget->window, NULL, NULL, NULL);

	return FALSE;
}
