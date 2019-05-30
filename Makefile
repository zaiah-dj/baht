# Makefile for this stupid script
CC=gcc
VERSION=0.02
CFLAGS=-Wall -Werror -Wno-unused -DSQROOGE_H -DLT_DEVICE=1 -DVERSION="\"$(VERSION)\"" #-DERRV_H
CC=clang
CFLAGS=-Wall -Werror -Wno-unused -DSQROOGE_H -DLT_DEVICE=1 -DVERSION="\"$(VERSION)\"" -DSEE_FRAMING #-DERRV_H
NAME=baht
DBSERVER="localhost"
DATABASE=ctrial_db
DBUSER=
DBPASSWORD=
SQLBIN=sqlcmd
LDFLAGS=-lgumbo -llua -lgnutls -lcurl
PREFIX=/usr/local

# top - Build the 'baht' scraper tool
top: vendor/single.o
	$(CC) $(CFLAGS) -DDEBUG vendor/single.o baht.c -L. $(LDFLAGS) -o $(NAME)

# insecure test
hr:
	make top && ./baht -u http://ramarcollins.com/test.html \
		--rootstart "div^root-main" \
		--jumpstart "div^jump-main" \
		--nodefile tests/ramartest.keys

# ...
hy:
	make top && ./baht -u http://ramarcollins.com/test.html --see-parsed-html

# file test
ff:
	make top && ./baht -f tests/unique.html \
		--rootstart "div^devil serial-mastermind xxx-uu col-md7" \
		--jumpstart "div^shaker tostada" \
		--nodefile tests/unique.keys

# href - Build baht's http handling code 
href: vendor/single.o
	$(CC) $(CFLAGS) -DIS_TEST vendor/single.o web.c -L. -lgnutls -lcurl -o ref && ./ref

# go - Run a typical query against a file
go: vendor/single.o
	./$(NAME) -l files/carribeanmotors.lua

# wop - Build the 'baht' scraper tool and run a dump 
wop: vendor/single.o
	$(CC) $(CFLAGS) vendor/single.o baht.c -L. $(LDFLAGS) -o $(NAME) && ./$(NAME) -f files/carri.html --show-full-key

#$(CC) $(CFLAGS) vendor/single.o baht.c -L. -lgumbo -o $(NAME) && ./$(NAME) -f files/carri.html

# explain - List all the targets and what they do
explain:
	@printf 'Available options are:\n'
	@sed -n '/^#/ { s/# //; 1d; p; }' Makefile | awk -F '-' '{ printf "  %-20s - %s\n", $$1, $$2 }'

# build - generic, useless build target
build:
	$(CC) $(CFLAGS) main.c -L. $(LDFLAGS) -o $(NAME) 

# echo - test the useless generic build 
echo:
	echo $(CC) $(CFLAGS) main.c -L. $(LDFLAGS) -o $(NAME) 

main:
	$(CC) $(CFLAGS) m.c -o mm

# clean - clean up
clean:
	-rm -f *.o vendor/*.o
	-rm -f $(NAME)

# mssql-load - Load SQL to MSSQL db
mssql-load: SQLBIN=sqlcmd
mssql-load: DBUSER=SA
mssql-load: DBPASSWORD=GKbAt68!
mssql-load:
	$(SQLBIN) -S $(DBSERVER) -i example.mssql -U $(DBUSER) -P $(DBPASSWORD)

# mysql-load - Load SQL to MySQL db
mysql-load: SQLBIN=mysql
mysql-load: DBUSER=root
mysql-load: DBPASSWORD=""
mysql-load:
	$(SQLBIN) -u $(DBUSER) --password=$(DBPASSWORD) < example.mysql

# listings - Make a database of listings
listings:
	printf ''


# install - Install the tool somewhere
install:
	test -d $(PREFIX)/bin/ || mkdir -p $(PREFIX)/bin/
	cp -v $(NAME) $(PREFIX)/bin/	


