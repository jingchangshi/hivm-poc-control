#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: HIVM_CONDA_ENV=... $0 command [args...]" >&2
  exit 2
fi

CONTAINER=s00653124_tilelang
USER_ID=s00653124
HOME_DIR=/home/s00653124
ENV_NAME="${HIVM_CONDA_ENV:-s00653124_tilelang}"

exec docker exec -i   -u "$USER_ID"   -e "HOME=$HOME_DIR"   -e "HIVM_CONDA_ENV=$ENV_NAME"   -w "$HOME_DIR"   "$CONTAINER"   bash -c '
    set -e
    source /data/pri/cann-9.2.0/cann/set_env.sh
    source /data/miniconda3/etc/profile.d/conda.sh
    conda activate "$HIVM_CONDA_ENV"
    exec "$@"
  ' _ "$@"
