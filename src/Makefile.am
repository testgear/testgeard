sbin_PROGRAMS = testgeard
pkglib_LTLIBRARIES = plugin.la
testgearddir = $(includedir)/testgear
testgeard_HEADERS = include/testgear/plugin.h

testgeard_SOURCES = connection-manager.c \
                    list.c \
                    main.c \
                    options.c \
                    plugin-manager.c \
                    daemon.c \
                    log.c \
                    message.c \
                    tcp.c \
                    include/testgear/list.h \
                    include/testgear/tcp.h \
                    include/testgear/daemon.h \
                    include/testgear/connection-manager.h \
                    include/testgear/signal.h \
                    include/testgear/debug.h \
                    include/testgear/options.h \
                    include/testgear/plugin.h \
                    include/testgear/plugin-manager.h \
                    include/testgear/message.h

testgeard_CFLAGS = -DSERVER -DPLUGINDIR=\"$(libdir)/testgear-plugins\" \
                            -DLOGDIR=\"$(localstatedir)/log/testgeard\"
testgeard_LDADD = -ldl

plugin_la_SOURCES = plugin.c
plugin_la_CFLAGS = -fPIC
plugin_la_LDFLAGS = -module -avoid-version -export-dynamic

bashcompletiondir=$(sysconfdir)/bash_completion.d
dist_bashcompletion_DATA=bash-completion/testgeard
