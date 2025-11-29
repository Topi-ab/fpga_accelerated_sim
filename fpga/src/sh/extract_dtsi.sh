#!/usr/bin/env bash
set -e

GIT_ROOT="$(git rev-parse --show-toplevel)"

# ---------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------

XSA_FILE="${GIT_ROOT}/fpga_proj/accelerator_top.xsa"
TMP_DIR="${GIT_ROOT}/tmp"
OUT_DIR="${GIT_ROOT}/fpga_out"
PROC_NAME="psu_cortexa53_0"
DTSI_DIR="${GIT_ROOT}/fpga/src/dtsi"
DT_REPO="${GIT_ROOT}/submodules/device-tree-xlnx"

# ---------------------------------------------------------------------

echo "=== Xilinx Device Tree + Bitstream Extract ==="
echo "Input  : $XSA_FILE"
echo "Output : $OUT_DIR"
echo "CPU    : $PROC_NAME"
echo "Repo   : $DT_REPO"
echo

# ---------------------------------------------------------------------
# Validate XSA file
# ---------------------------------------------------------------------
if [ ! -f "$XSA_FILE" ]; then
  echo "ERROR: $XSA_FILE not found!"
  exit 1
fi

mkdir -p "$OUT_DIR"
mkdir -p "$TMP_DIR"

# ---------------------------------------------------------------------
# Extract .bit from XSA, if any
# ---------------------------------------------------------------------
if unzip -l "$XSA_FILE" | grep -q "\.bit"; then
  echo "Extracting bitstream from XSA..."
  unzip -j "$XSA_FILE" "*.bit" -d "$OUT_DIR" >/dev/null

  # Normalize name → accelerator_top.bit.bin
  BIT=$(ls "$OUT_DIR"/*.bit | head -n1)
  mv "$BIT" "$OUT_DIR/accelerator_top.bit.bin"

  echo "Bitstream extracted: $OUT_DIR/accelerator_top.bit.bin"
else
  echo "No embedded .bit found in XSA."
  exit 1
fi

# ---------------------------------------------------------------------
# Generate device-tree sources via XSCT
# ---------------------------------------------------------------------
echo "Generating device tree from XSA with XSCT..."

set +e  # we want to capture XSCT exit code explicitly
xsct -eval "
  hsi open_hw_design $XSA_FILE;
  hsi set_repo_path $DT_REPO;
  hsi create_sw_design device_tree -os device_tree -proc $PROC_NAME;
  hsi set_property CONFIG.dt_overlay true [hsi get_os device_tree];
  hsi generate_target -dir $TMP_DIR;
  exit;
"
XSCT_RC=$?
set -e

if [ $XSCT_RC -ne 0 ]; then
  echo "ERROR: XSCT (HSI) failed with exit code $XSCT_RC"
  exit 1
fi

dtc -@ -I dts -O dtb -o "$OUT_DIR/accelerator_top.dtbo" $DTSI_DIR/accelerator_top.dts

echo "DTBO creation done"

echo "Copy these two files to the Kria module:"
ls -lrth $OUT_DIR

exit 0
