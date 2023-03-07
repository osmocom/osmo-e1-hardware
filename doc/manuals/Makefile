OSMO_GSM_MANUALS_DIR:=$(shell ./osmo-gsm-manuals-dir.sh)

srcdir = .

ASCIIDOC = icE1usb-usermanual.adoc e1-tracer-usermanual.adoc
include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.asciidoc.inc
icE1usb-usermanual.pdf: chapters/icE1usb/*.adoc
e1-tracer-usermanual.pdf: chapters/e1-tracer/*.adoc

OSMO_REPOSITORY = osmo-e1-hardware
include $(OSMO_GSM_MANUALS_DIR)/build/Makefile.common.inc
