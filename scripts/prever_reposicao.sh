#!/usr/bin/env bash
#
# KAGE Home Supply - Previsao de Reposicao
# Autor: Anderson S. Nogueira
# Versao: 1.0.0
#
# Changelog:
# - 1.0.0: Calculo de autonomia e alerta de estoque minimo em CSV.

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CSV_FILE="${PROJECT_ROOT}/data/estoque.csv"

fail() {
  echo "Erro: $*" >&2
  exit 1
}

main() {
  [[ -f "${CSV_FILE}" ]] || fail "arquivo CSV nao encontrado: ${CSV_FILE}"

  echo "Previsao de reposicao - KAGE Home Supply"
  echo "Arquivo: ${CSV_FILE}"
  echo

  awk -F',' '
    NR == 1 { next }
    {
      item=$1; categoria=$2; quantidade=$3 + 0; unidade=$4; consumo=$5 + 0; minimo=$8 + 0
      gsub(/^"|"$/, "", item)
      gsub(/^"|"$/, "", categoria)
      gsub(/^"|"$/, "", unidade)
      autonomia = consumo > 0 ? quantidade / consumo : 0
      status = quantidade <= minimo ? "CRITICO" : "OK"
      printf "%-28s | %-14s | %8.2f %-8s | autonomia: %6.2f mes(es) | %s\n", item, categoria, quantidade, unidade, autonomia, status
    }
  ' "${CSV_FILE}"
}

main "$@"
