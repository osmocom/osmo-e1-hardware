OSMO_GSM_MANUALS_DIR:=$(shell ./osmo-gsm-manuals-dir.sh)

srcdir = .

ASCIIDOC = icE1usb-usermanual.adoc
include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.asciidoc.inc
icE1usb-usermanual.pdf: chapters/icE1usb/*.adoc

include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.common.inc
