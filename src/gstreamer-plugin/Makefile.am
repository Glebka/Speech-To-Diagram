# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstspeechrecognition.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstspeechrecognition_la_SOURCES = gstpocketsphinx.c gstpocketsphinx.h psmarshal.h psmarshal.c gstvader.c gstvader.h gstgooglespeech.c gstgooglespeech.h googmarshal.h googmarshal.c
#gstgooglespeech.c gstgooglespeech.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstspeechrecognition_la_CFLAGS = $(GST_CFLAGS)
libgstspeechrecognition_la_LIBADD = $(GST_LIBS) -lgstaudio-1.0 -lsphinxbase -lpocketsphinx -lcurl -ljson-glib-1.0
libgstspeechrecognition_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstspeechrecognition_la_LIBTOOLFLAGS = --tag=disable-static

INCLUDES = -I/usr/include/sphinxbase -I/usr/include/pocketsphinx -I/usr/include/json-glib-1.0

# headers we need but don't want installed
noinst_HEADERS = gstpocketsphinx.h psmarshal.h gstvader.h gstgooglespeech.h googmarshal.h

psmarshal.h: psmarshal.list
	glib-genmarshal --header --prefix=ps_marshal $(srcdir)/psmarshal.list > psmarshal.h.tmp
	mv psmarshal.h.tmp psmarshal.h

psmarshal.c: psmarshal.list
	echo "#include \"glib-object.h\"" >> psmarshal.c.tmp
	echo "#include \"psmarshal.h\"" >> psmarshal.c.tmp
	glib-genmarshal --body --prefix=ps_marshal $(srcdir)/psmarshal.list >> psmarshal.c.tmp
	mv psmarshal.c.tmp psmarshal.c

googmarshal.h: googmarshal.list
	glib-genmarshal --header --prefix=goog_marshal $(srcdir)/googmarshal.list > googmarshal.h.tmp
	mv googmarshal.h.tmp googmarshal.h

googmarshal.c: googmarshal.list
	echo "#include \"glib-object.h\"" >> googmarshal.c.tmp
	echo "#include \"googmarshal.h\"" >> googmarshal.c.tmp
	glib-genmarshal --body --prefix=goog_marshal $(srcdir)/googmarshal.list >> googmarshal.c.tmp
	mv googmarshal.c.tmp googmarshal.c