
SRCS=$(wildcard *.mu)

OBJS=$(SRCS:.mu=.html)

%.html: %.mu
	@ echo Mupping $<...
	@ mup.wrapper html $<

.PHONY: all clean install

all: $(OBJS)

upload: all
	@ ln -sf OVERVIEW.html index.html
	@ rsync -L mup.css MANUAL.html index.html *.png ssh.tuxfamily.org:/home/non/non-mixer.tuxfamily.org-web/htdocs
	@ rm -f index.html

install:
	@ mkdir -p $(DESTDIR)$(DOCUMENT_PATH)/non-mixer
	@ cp $(OBJS) *.png mup.css ../../COPYING $(DESTDIR)$(DOCUMENT_PATH)/non-mixer
	@ chmod 644 $(DESTDIR)$(DOCUMENT_PATH)/non-mixer/*
#	@ ln -sf $(PIXMAP_PATH)/logo.png $(DOCUMENT_PATH)

clean:
	rm -f $(OBJS)
