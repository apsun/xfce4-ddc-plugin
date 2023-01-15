AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-ddc-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

plugin_LTLIBRARIES = \
	libddcplugin.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libddcplugin_built_sources = \
	ddcplugin_settings_dialog_ui.h

libddcplugin_la_SOURCES = \
	ddcplugin.c \
	ddcplugin_display.c \
	ddcplugin_display.h \
	ddcplugin_settings.c \
	ddcplugin_settings.h \
	ddcplugin_settings_dialog.c \
	ddcplugin_settings_dialog.h

libddcplugin_la_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(KEYBINDER_CFLAGS) \
	$(DDCUTIL_CFLAGS) \
	$(PLATFORM_CFLAGS)

libddcplugin_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libddcplugin_la_LIBADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(KEYBINDER_LIBS) \
	$(DDCUTIL_LIBS)

desktopdir = \
	$(datadir)/xfce4/panel/plugins

desktop_DATA = \
	ddcplugin.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	ddcplugin.desktop.in \
	ddcplugin_settings_dialog.glade

DISTCLEANFILES = \
	$(desktop_DATA)

if MAINTAINER_MODE

BUILT_SOURCES = \
	$(libddcplugin_built_sources)

DISTCLEANFILES += \
	$(libddcplugin_built_sources)

ddcplugin_settings_dialog_ui.h: ddcplugin_settings_dialog.glade
	$(AM_V_GEN) xdt-csource --static --strip-comments --strip-content --name=ddcplugin_settings_dialog_ui $< >$@

endif
