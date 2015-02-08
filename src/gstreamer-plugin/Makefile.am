# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstpocketsphinx.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstpocketsphinx_la_SOURCES = gstpocketsphinx.c gstpocketsphinx.h psmarshal.h psmarshal.c gstvader.c gstvader.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstpocketsphinx_la_CFLAGS = $(GST_CFLAGS)
libgstpocketsphinx_la_LIBADD = $(GST_LIBS) -lgstaudio-1.0 -lsphinxbase -lpocketsphinx
libgstpocketsphinx_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstpocketsphinx_la_LIBTOOLFLAGS = --tag=disable-static

INCLUDES = -I/usr/include/sphinxbase -I/usr/include/pocketsphinx

# headers we need but don't want installed
noinst_HEADERS = gstpocketsphinx.h psmarshal.h gstvader.h

psmarshal.h: psmarshal.list
	glib-genmarshal --header --prefix=ps_marshal $(srcdir)/psmarshal.list > psmarshal.h.tmp
	mv psmarshal.h.tmp psmarshal.h

psmarshal.c: psmarshal.list
	echo "#include \"glib-object.h\"" >> psmarshal.c.tmp
	echo "#include \"psmarshal.h\"" >> psmarshal.c.tmp
	glib-genmarshal --body --prefix=ps_marshal $(srcdir)/psmarshal.list >> psmarshal.c.tmp
	mv psmarshal.c.tmp psmarshal.c