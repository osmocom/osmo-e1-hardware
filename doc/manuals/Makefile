OSMO_GSM_MANUALS_DIR:=$(shell pkg-config osmo-gsm-manuals --variable=osmogsmmanualsdir)

sdcdir = .

ASCIIDOC = icE1usb-usermanual.adoc
include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.asciidoc.inc
icE1usb-usermanual.pdf: chapters/*.adoc

include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.common.inc
