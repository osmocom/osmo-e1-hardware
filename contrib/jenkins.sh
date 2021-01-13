#!/usr/bin/env bash
# jenkins build helper script for osmo-e1-hardware.  This is how we build on jenkins.osmocom.org
#
# environment variables:
# * WITH_MANUALS: build manual PDFs if set to "1"
# * PUBLISH: upload manuals after building if set to "1" (ignored without WITH_MANUALS = "1")

# ugly, ugly hack to work around the fact that we cannot _extend_ the path when executing
# a docker container.  We can either override it (and loose our /opt/... toolchain paths)
# or we can not specify the /build_bin whcih means the osmo-build-dep.sh is not found
# See https://osmocom.org/issues/4911
if [ -d /build_bin ]; then
	PATH=$PATH:/build_bin
	export PATH
fi

if ! [ -x "$(command -v osmo-build-dep.sh)" ]; then
	echo "Error: We need to have scripts/osmo-deps.sh from http://git.osmocom.org/osmo-ci/ in PATH !"
	exit 2
fi

set -e

TOPDIR=`pwd`
publish="$1"

base="$PWD"
deps="$base/deps"
inst="$deps/install"
export deps inst

export PKG_CONFIG_PATH="$inst/lib/pkgconfig:$PKG_CONFIG_PATH"
export LD_LIBRARY_PATH="$inst/lib"
export PATH="$inst/bin:$PATH"

osmo-clean-workspace.sh

mkdir "$deps" || true

# we assume that PATH includes the path to the respective toolchain

# firmware build
FW_DIRS="firmware/ice40-riscv/e1-tracer firmware/ice40-riscv/icE1usb"
for d in $FW_DIRS; do
	echo
	echo "=============== $d FIRMWARE  =============="
	make -C $d clean all
done

# The argument '--publish' is used to trigger publication/upload of firmware
if [ "x$publish" = "x--publish" ]; then

	echo
	echo "=============== UPLOAD FIRMWARE =============="

	cat > "/build/known_hosts" <<EOF
[rita.osmocom.org]:48 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDDgQ9HntlpWNmh953a2Gc8NysKE4orOatVT1wQkyzhARnfYUerRuwyNr1GqMyBKdSI9amYVBXJIOUFcpV81niA7zQRUs66bpIMkE9/rHxBd81SkorEPOIS84W4vm3SZtuNqa+fADcqe88Hcb0ZdTzjKILuwi19gzrQyME2knHY71EOETe9Yow5RD2hTIpB5ecNxI0LUKDq+Ii8HfBvndPBIr0BWYDugckQ3Bocf+yn/tn2/GZieFEyFpBGF/MnLbAAfUKIdeyFRX7ufaiWWz5yKAfEhtziqdAGZaXNaLG6gkpy3EixOAy6ZXuTAk3b3Y0FUmDjhOHllbPmTOcKMry9
[rita.osmocom.org]:48 ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBPdWn1kEousXuKsZ+qJEZTt/NSeASxCrUfNDW3LWtH+d8Ust7ZuKp/vuyG+5pe5pwpPOgFu7TjN+0lVjYJVXH54=
[rita.osmocom.org]:48 ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIK8iivY70EiR5NiGChV39gRLjNpC8lvu1ZdHtdMw2zuX
EOF
	SSH_COMMAND="ssh -o 'UserKnownHostsFile=/build/known_hosts' -p 48"
	rsync --archive --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/icE1usb/*-*-*-*.{bin,elf} binaries@rita.osmocom.org:web-files/icE1usb/firmware/all/
	rsync --archive --copy-links --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/icE1usb/icE1usb-fw.{bin,elf} binaries@rita.osmocom.org:web-files/icE1usb/firmware/latest/
	rsync --archive --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/e1-tracer/*-*-*-*.{bin,elf} binaries@rita.osmocom.org:web-files/e1-tracer/firmware/all/
	rsync --verbose --copy-links --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/e1-tracer/e1_tracer-fw.{bin,elf} binaries@rita.osmocom.org:web-files/e1-tracer/firmware/latest/
fi

# manuals build + optional publication
if [ "$WITH_MANUALS" = "1" ]; then
	make -C doc/manuals clean all
	if [ "$PUBLISH" = "1" ]; then
		make -C doc/manuals publish
	fi
fi

# gateware build
if [ "$WITH_GATEWARE" = "1" ]; then
	GATE_VARS="IGNORE_TIMING=1 SINGLE_CHANNEL=1"
	GATE_DIRS="gateware/e1-tracer gateware/icE1usb"
	for d in $GATE_DIRS; do
		echo
		echo "=============== $d GATEWARE  =============="
		make -C $d clean
		make -C $d ${GATE_VARS}
	done
fi

osmo-clean-workspace.sh
