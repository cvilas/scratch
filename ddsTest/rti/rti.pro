TEMPLATE = subdirs
SUBDIRS = messenger plant monitor
CONFIG += ordered

monitor.depends = messenger
plant.depends = messenger

OTHER_FILES += notes.txt todo.txt
