sbin_PROGRAMS = testgeard
pkglib_LIBRARIES = libplugin.a
pkginclude_HEADERS = include/testgear/plugin.h
testgeard_SOURCES = connection-manager.c \
                    list.c \
                    main.c \
                    options.c \
                    plugin-manager.c \
                    daemon.c \
                    log.c \
                    message.c \
                    tcp.c
testgeard_CFLAGS = -DSERVER
testgeard_LDADD = -ldl
libplugin_a_SOURCES = plugin.c

bashcompletiondir=$(sysconfdir)/bash_completion.d
dist_bashcompletion_DATA=bash-completion/testgeard
