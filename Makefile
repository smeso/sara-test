#!/usr/bin/make
#
#     sara-test - S.A.R.A.'s test suite
#     Copyright (C) 2017  Salvatore Mesoraca <s.mesoraca16@gmail.com>
#
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifndef DESTDIR
DESTDIR := /
endif
ifndef BINDIR
BINDIR := usr/bin/
endif
ifndef EXTRA_BINS_PATH
EXTRA_BINS_PATH := /usr/lib/sara-test/
endif

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
CFLAGS_nopie := -O2 $(CFLAGS)
LDFLAGS_nopie := -fno-pie $(LDFLAGS)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e no-pie),)
	LDFLAGS_nopie := -no-pie $(LDFLAGS_nopie)
endif
CFLAGS := -O2 -fPIE -fstack-protector $(CFLAGS)
LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-Bsymbolic-functions -pie $(LDFLAGS)
BIN := ./bin/
SOURCE := ./src/

all:	$(BIN) \
	$(BIN)sara-test \
	$(BIN)trampoline \
	$(BIN)trampoline_nopie \
	$(BIN)procattr \
	$(BIN)fake_tramp \
	$(BIN)transfer

$(BIN):
	mkdir -p $(BIN)

$(SOURCE)%.o: $(SOURCE)%.c
	$(CC) -c -o $@ $< $(CFLAGS) -DEXTRA_BINS_PATH=\"${EXTRA_BINS_PATH}\"

$(SOURCE)libsara/libsara.o: $(SOURCE)libsara/libsara.c
	export CROSS_COMPILE
	cd $(SOURCE)/libsara && $(MAKE) libsara.o

$(BIN)sara-test: $(SOURCE)sara-test.o $(SOURCE)utils.o $(SOURCE)libsara/libsara.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN)procattr: $(SOURCE)procattr.o $(SOURCE)utils.o $(SOURCE)libsara/libsara.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN)transfer: $(SOURCE)transfer.o $(SOURCE)libsara/libsara.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN)trampoline: $(SOURCE)trampoline.o
	$(CC) -z execstack -o $@ $^ $(LDFLAGS)

$(SOURCE)trampoline_nopie.o: $(SOURCE)trampoline.c
	$(CC) -c -o $@ $< $(CFLAGS_nopie) -DEXTRA_BINS_PATH=\"${EXTRA_BINS_PATH}\"

$(BIN)trampoline_nopie: $(SOURCE)trampoline_nopie.o
	$(CC) -z execstack -o $@ $^ $(LDFLAGS_nopie)

$(SOURCE)fake_tramp.o: $(SOURCE)fake_tramp.c
	$(CC) -c -o $@ $< -Wno-pointer-to-int-cast $(CFLAGS_nopie)

$(BIN)fake_tramp: $(SOURCE)fake_tramp.o
	$(CC) -z noexecstack -o $@ $^ $(LDFLAGS_nopie)

ifdef DESTDIR
ifdef BINDIR
ifdef EXTRA_BINS_PATH
install: all
	mkdir -p $(DESTDIR)/$(EXTRA_BINS_PATH)
	mkdir -p $(DESTDIR)/$(BINDIR)
	mkdir -p $(DESTDIR)/usr/share/man/man1/
	cp $(BIN)trampoline $(DESTDIR)/$(EXTRA_BINS_PATH)
	cp $(BIN)trampoline_nopie $(DESTDIR)/$(EXTRA_BINS_PATH)
	cp $(BIN)procattr $(DESTDIR)/$(EXTRA_BINS_PATH)
	cp $(BIN)transfer $(DESTDIR)/$(EXTRA_BINS_PATH)
	cp $(BIN)fake_tramp $(DESTDIR)/$(EXTRA_BINS_PATH)
	cp $(BIN)sara-test $(DESTDIR)/$(BINDIR)
	chmod 755 $(DESTDIR)/$(BINDIR)/sara-test
	chmod 755 $(DESTDIR)/$(EXTRA_BINS_PATH)/trampoline
	chmod 755 $(DESTDIR)/$(EXTRA_BINS_PATH)/trampoline_nopie
	chmod 755 $(DESTDIR)/$(EXTRA_BINS_PATH)/transfer
	chmod 755 $(DESTDIR)/$(EXTRA_BINS_PATH)/procattr
	chmod 755 $(DESTDIR)/$(EXTRA_BINS_PATH)/fake_tramp
	mkdir -p $(DESTDIR)/etc/sara/wxprot.conf.d/
	echo "/$(BINDIR)sara-test mprotect,verbose" > $(DESTDIR)/etc/sara/wxprot.conf.d/10_saratest.conf
	echo "$(EXTRA_BINS_PATH)* mprotect,verbose" >> $(DESTDIR)/etc/sara/wxprot.conf.d/10_saratest.conf
	echo "$(EXTRA_BINS_PATH)procattr mmap,other,complain,verbose" >> $(DESTDIR)/etc/sara/wxprot.conf.d/10_saratest.conf
	echo "$(EXTRA_BINS_PATH)fake_tramp mprotect,emutramp_or_mprotect,verbose" >> $(DESTDIR)/etc/sara/wxprot.conf.d/10_saratest.conf
	echo "$(EXTRA_BINS_PATH)trampoline* mprotect,emutramp_or_mprotect,verbose" >> $(DESTDIR)/etc/sara/wxprot.conf.d/10_saratest.conf
	gzip -c man/sara-test.1 > $(DESTDIR)/usr/share/man/man1/sara-test.1.gz
uninstall:
	-rm $(DESTDIR)/$(EXTRA_BINS_PATH)/trampoline
	-rm $(DESTDIR)/$(EXTRA_BINS_PATH)/trampoline_nopie
	-rm $(DESTDIR)/$(EXTRA_BINS_PATH)/procattr
	-rm $(DESTDIR)/$(EXTRA_BINS_PATH)/transfer
	-rm $(DESTDIR)/$(EXTRA_BINS_PATH)/fake_tramp
	-rm $(DESTDIR)/$(BINDIR)/sara-test
	-rm $(DESTDIR)/usr/share/man/man1/sara-test.1.gz
endif
endif
endif

clean:
	-rm -f $(SOURCE)*.o $(SOURCE)*~
	-rm -f *.o *~
	cd $(SOURCE)/libsara && $(MAKE) clean

distclean: clean
	-rm -f $(BIN)*
	cd $(SOURCE)/libsara && $(MAKE) distclean

.PHONY: all install uninstall clean distclean
