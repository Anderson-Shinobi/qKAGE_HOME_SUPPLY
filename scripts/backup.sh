#!/usr/bin/env bash
#
# KAGE Home Supply - Backup Operacional
# Autor: Anderson S. Nogueira
# Versao: 1.0.0
#
# Changelog:
# - 1.0.0: Backup versionado do CSV de estoque.

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CSV_FILE="${PROJECT_ROOT}/data/estoque.csv"
BACKUP_DIR="${PROJECT_ROOT}/backups"

fail() {
  echo "Erro: $*" >&2
  exit 1
}

main() {
  [[ -f "${CSV_FILE}" ]] || fail "arquivo CSV nao encontrado: ${CSV_FILE}"

  mkdir -p "${BACKUP_DIR}"

  local timestamp
  timestamp="$(date '+%Y%m%d_%H%M%S')"
  local target="${BACKUP_DIR}/estoque_${timestamp}.csv"

  cp "${CSV_FILE}" "${target}"

  echo "Backup criado com sucesso."
  echo "Origem: ${CSV_FILE}"
  echo "Destino: ${target}"
}

main "$@"
