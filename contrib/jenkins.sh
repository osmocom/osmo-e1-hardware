#!/usr/bin/env bash
# jenkins build helper script for osmo-e1-hardware.  This is how we build on jenkins.osmocom.org
#
# environment variables:
# * PUBLISH: upload manuals after building if set to "1" (ignored without WITH_MANUALS = "1")
# * JOB_TYPE: one of "firmware", "gateware", "manuals", "software"
#
# We assume that PATH includes the path to the respective toolchain.
# The argument '--publish' is used to trigger publication/upload of firmware.

set -e

TOPDIR=`pwd`
publish="$1"

cat > "/build/known_hosts" <<EOF
[ftp.osmocom.org]:48 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDDgQ9HntlpWNmh953a2Gc8NysKE4orOatVT1wQkyzhARnfYUerRuwyNr1GqMyBKdSI9amYVBXJIOUFcpV81niA7zQRUs66bpIMkE9/rHxBd81SkorEPOIS84W4vm3SZtuNqa+fADcqe88Hcb0ZdTzjKILuwi19gzrQyME2knHY71EOETe9Yow5RD2hTIpB5ecNxI0LUKDq+Ii8HfBvndPBIr0BWYDugckQ3Bocf+yn/tn2/GZieFEyFpBGF/MnLbAAfUKIdeyFRX7ufaiWWz5yKAfEhtziqdAGZaXNaLG6gkpy3EixOAy6ZXuTAk3b3Y0FUmDjhOHllbPmTOcKMry9
[ftp.osmocom.org]:48 ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBPdWn1kEousXuKsZ+qJEZTt/NSeASxCrUfNDW3LWtH+d8Ust7ZuKp/vuyG+5pe5pwpPOgFu7TjN+0lVjYJVXH54=
[ftp.osmocom.org]:48 ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIK8iivY70EiR5NiGChV39gRLjNpC8lvu1ZdHtdMw2zuX
EOF

case "$JOB_TYPE" in
"firmware")
	FW_DIRS="firmware/ice40-riscv/e1-tracer firmware/ice40-riscv/icE1usb"
	for d in $FW_DIRS; do
		echo
		echo "=============== $d FIRMWARE  =============="
		make -C $d clean all
	done

	if [ "x$publish" = "x--publish" ]; then
		echo
		echo "=============== UPLOAD FIRMWARE =============="
		SSH_COMMAND="ssh -o 'UserKnownHostsFile=/build/known_hosts' -p 48"
		rsync --archive --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/icE1usb/icE1usb-fw-*.{bin,elf} binaries@ftp.osmocom.org:web-files/icE1usb/firmware/all/
		rsync --archive --copy-links --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/icE1usb/icE1usb-fw.{bin,elf} binaries@ftp.osmocom.org:web-files/icE1usb/firmware/latest/
		rsync --archive --verbose --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/e1-tracer/e1_tracer-fw-*.{bin,elf} binaries@ftp.osmocom.org:web-files/e1-tracer/firmware/all/
		rsync --verbose --copy-links --compress --rsh "$SSH_COMMAND" $TOPDIR/firmware/ice40-riscv/e1-tracer/e1_tracer-fw.{bin,elf} binaries@ftp.osmocom.org:web-files/e1-tracer/firmware/latest/
	fi
	;;
"gateware")
	GATE_VARS="IGNORE_TIMING=1 SINGLE_CHANNEL=1"
	GATE_DIRS="gateware/e1-tracer gateware/icE1usb"
	for d in $GATE_DIRS; do
		echo
		echo "=============== $d GATEWARE  =============="
		make -C $d clean
		make -C $d ${GATE_VARS}
	done
	;;
"manuals")
	make -C doc/manuals clean all

	if [ "$PUBLISH" = "1" ]; then
		make -C doc/manuals publish
	fi
	;;
"software")
	if ! [ -x "$(command -v osmo-build-dep.sh)" ]; then
		echo "Error: We need to have scripts/osmo-deps.sh from http://git.osmocom.org/osmo-ci/ in PATH !"
		exit 2
	fi

	base="$PWD"
	deps="$base/deps"
	inst="$deps/install"
	export deps inst

	export PKG_CONFIG_PATH="$inst/lib/pkgconfig:$PKG_CONFIG_PATH"
	export LD_LIBRARY_PATH="$inst/lib"
	export PATH="$inst/bin:$PATH"

	osmo-clean-workspace.sh

	mkdir -p "$deps"

	osmo-build-dep.sh libosmocore "" ac_cv_path_DOXYGEN=false
	osmo-build-dep.sh libosmo-abis

	SW_DIRS="software/e1-tracer/record software/e1-tracer/analyze"
	for d in $SW_DIRS; do
		echo
		echo "=============== $d SOFTWARE =============="
		make -C $d clean all
	done

	osmo-clean-workspace.sh
	;;
*)
	echo "ERROR: unsupported value for JOB_TYPE: $JOB_TYPE"
	exit 1
esac
